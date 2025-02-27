# Watchdog Processor Simulation in C

This project simulates a **Watchdog Processor** using C. A watchdog processor monitors the execution of a system, ensuring it continues to operate correctly by supervising tasks and detecting anomalies or faults.

## Features
- Simulates a watchdog processor to oversee system tasks.
- Detects faults or failures in the system.
.

## Files
- **main.c**: The main entry point of the simulation.
- **backup.c**: A backup of the main source code.
- **.vscode/**: Configuration files for Visual Studio Code.
- **.gitignore**: Lists files and directories to be ignored by Git.
- **LICENSE**: License information for the project.

## Prerequisites
To compile and run this project, you need:
- A C compiler (e.g., GCC)
- Make (optional, for build automation)

## Compilation and Execution
To compile and run the simulation:  
gcc main.c -o watchdog_processor_sim  
./watchdog_processor_sim  


## Usage
- The simulation monitors tasks within a system and detects anomalies.
- Upon detecting a fault, the watchdog processor takes corrective action.
- Adjust parameters in `main.c` to customize the processor's behavior.

## License
This project is licensed under the terms specified in the `LICENSE` file.

## Contribution
Contributions are welcome! Feel free to open issues or submit pull requests.

## Author
- [Sadegh](https://github.com/sadegh15khedry)

## Acknowledgments
- Inspired by hardware watchdog processors used in critical systems.
