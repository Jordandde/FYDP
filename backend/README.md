# Getting started
The backend of our project uses C++ to pass matrices from the frontend to the TCP connected FPGA. 
This is done via JSON from the frontend to backend and an ethernet connection from the backend to the FPGA.

## Objective of the backend
The backend should do 4 things:
1. Start up in the background when the desktop app icon is clicked
2. Listen for a JSON message, and then process the message when it is received
3. Package the received data into a useable package that can be attached to the TCP call and sent to the FPGA
4. Listen and wait for a response from the FPGA

## Precautions
1. Make sure to add a timeout while the backend is listening to the FPGA to avoid hanging the whole application
2. Add a calibration step as environment conditions introduce a (supposedly) linear error in multiplications


## Technology
The backend should consist of a simple http server. C++ will be used for this, largely for the sake of speed and being able to work effectively in parallel.

## Building
Make sure you have xcode command line tools installed on Mac OS, as well as boost:

`xcode-select --install`

`brew install boost`

`clang++ -Wall -std=c++17 -o main main.cpp -lboost_system -lboost_program_options && ./main`

## Running
`./main`

## Command-line Arguments
The `main` program accepts the following command-line arguments:

- `--help`: Produces a help message that lists all the available command-line arguments.

- `--no_frontend`: If this flag is set, the front-end is not included in the loop and fake input is used instead.

- `--no_fpga`: If this flag is set, the FPGA is not included in the loop and fake output is used instead.

For example, to run the program with no frontend and no fpga, you would use:

`./main --no_frontend --no_fpga`
