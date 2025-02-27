#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

// Define program states
typedef enum {
    STATE_START,
    STATE_SORT_PREPARE,
    STATE_FIND_MIN,
    STATE_SWAP,
    STATE_NEXT_ITERATION,
    STATE_FINISHED,
    STATE_ERROR
} program_state_t;

#define ARRAY_SIZE 10
#define TIMEOUT 5  // Watchdog timeout in seconds

volatile program_state_t current_state = STATE_START;  // Shared state variable
time_t last_state_change;                              // Time of last state change

// Function to check if the transition is valid
bool is_valid_transition(program_state_t from, program_state_t to) {
    switch (from) {
        case STATE_START:
            return to == STATE_SORT_PREPARE;
        case STATE_SORT_PREPARE:
            return to == STATE_FIND_MIN || to == STATE_ERROR;
        case STATE_FIND_MIN:
            return to == STATE_SWAP || to == STATE_ERROR;
        case STATE_SWAP:
            return to == STATE_NEXT_ITERATION || to == STATE_ERROR;
        case STATE_NEXT_ITERATION:
            return to == STATE_FIND_MIN || to == STATE_FINISHED || to == STATE_ERROR;
        case STATE_FINISHED:
            return false;  // No transitions allowed from FINISHED
        case STATE_ERROR:
            return false;  // No transitions allowed from ERROR
        default:
            return false;
    }
}

// Function to perform selection sort with state transitions
void* main_program(void* arg) {
    int* array = (int*)arg;

    int cpu = sched_getcpu();
    printf("Main program started on core %d\n", cpu);

    // Transition to SORT_PREPARE state
    current_state = STATE_SORT_PREPARE;
    last_state_change = time(NULL);
    sleep(1);  // Simulate preparation work

    for (int i = 0; i < ARRAY_SIZE - 1; i++) {
        // Transition to FIND_MIN state
        current_state = STATE_FIND_MIN;
        last_state_change = time(NULL);
        sleep(1);  // Simulate find min work

        int min_idx = i;
        for (int j = i + 1; j < ARRAY_SIZE; j++) {
            if (array[j] < array[min_idx]) {
                min_idx = j;
            }
        }

        // Transition to SWAP state
        current_state = STATE_SWAP;
        last_state_change = time(NULL);
        sleep(1);  // Simulate swap preparation

        // Injected invalid transition 
        // sleep(1);
        // current_state = STATE_SORT_PREPARE;  // Invalid transition
        // last_state_change = time(NULL);
        // sleep(1);

        int temp = array[min_idx];
        array[min_idx] = array[i];
        array[i] = temp;
        sleep(1);  // Simulate swap work

        // Transition to NEXT_ITERATION state
        current_state = STATE_NEXT_ITERATION;
        last_state_change = time(NULL);
        sleep(1);  // Simulate iteration transition
    }

    // Transition to FINISHED state
    current_state = STATE_FINISHED;
    last_state_change = time(NULL);

    cpu = sched_getcpu();
    printf("Main program finished on core %d\n", cpu);

    return NULL;
}

// Watchdog thread
void* watchdog(void* arg) {
    int cpu = sched_getcpu();
    printf("Watchdog started on core %d\n", cpu);

    program_state_t last_state = current_state;

    while (1) {
        time_t now = time(NULL);

        if (current_state != last_state) {
            // Check for invalid state transitions only when state changes
            if (!is_valid_transition(last_state, current_state)) {
                printf("Watchdog: ERROR - Invalid state transition from %d to %d.\n", last_state, current_state);
                current_state = STATE_ERROR;
                exit(1);
            }
            // Update last_state since state has changed
            last_state = current_state;
        }

        // Monitor state durations
        switch (current_state) {
            case STATE_START:
            case STATE_SORT_PREPARE:
            case STATE_FIND_MIN:
            case STATE_SWAP:
            case STATE_NEXT_ITERATION:
                if (difftime(now, last_state_change) > TIMEOUT) {
                    printf("Watchdog: ERROR - Main program stuck in state %d.\n", current_state);
                    current_state = STATE_ERROR;
                    exit(1);
                }
                break;

            case STATE_FINISHED:
                printf("Watchdog: Main program successfully finished on core %d.\n", sched_getcpu());
                pthread_exit(NULL);  // Exit the watchdog gracefully
                break;

            case STATE_ERROR:
                printf("Watchdog: Main program encountered an error.\n");
                exit(1);

            default:
                printf("Watchdog: Unknown state detected.\n");
                current_state = STATE_ERROR;
                exit(1);
        }

        printf("Watchdog: Monitoring state: %d on core %d\n", current_state, sched_getcpu());
        sleep(1);  // Periodic monitoring
    }
    return NULL;
}

int main() {
    pthread_t thread_main, thread_watchdog;
    cpu_set_t cpu_set_main, cpu_set_watchdog;

    // Initialize the array for selection sort
    int array[ARRAY_SIZE] = {64, 34, 25, 12, 22, 11, 90, 88, 76, 54};
    printf("Initial array:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    // Record the initial state change timestamp
    last_state_change = time(NULL);

    // Create the main program thread
    pthread_create(&thread_main, NULL, main_program, array);
    CPU_ZERO(&cpu_set_main);
    CPU_SET(0, &cpu_set_main);  // Bind main program to core 0
    pthread_setaffinity_np(thread_main, sizeof(cpu_set_t), &cpu_set_main);

    // Create the watchdog thread
    pthread_create(&thread_watchdog, NULL, watchdog, NULL);
    CPU_ZERO(&cpu_set_watchdog);
    CPU_SET(1, &cpu_set_watchdog);  // Bind watchdog to core 1
    pthread_setaffinity_np(thread_watchdog, sizeof(cpu_set_t), &cpu_set_watchdog);

    // Wait for the main program to finish
    pthread_join(thread_main, NULL);

    // Print the sorted array
    printf("Sorted array:\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    // Exit the watchdog thread gracefully
    pthread_cancel(thread_watchdog);
    pthread_join(thread_watchdog, NULL);

    return 0;
}
