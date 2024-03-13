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

## Autoformatting
Install clang-format using `brew install clang-format`

Run using `clang-format --style=file -i *.cpp *.hpp`

## Building
Make sure you have xcode command line tools installed on Mac OS, as well as boost:

`xcode-select --install`

`brew install boost`

Intel:
`clang++ -Wall -std=c++17 -o main src/*.cpp -I include -lboost_system -lboost_program_options`

Apple Silicon:
`clang++ -Wall -std=c++17 -o main src/*.cpp -I include -I/opt/homebrew/include -L/opt/homebrew/lib -lboost_system -lboost_program_options`

If you receive errors related to boost not being found, try running `brew list boost` and adding the `-I` and `-L` flags with the locations of the include and lib directories shown.

## Running
`./main`

## Command-line Arguments
The `main` program accepts the following command-line arguments:

- `--help`: Produces a help message that lists all the available command-line arguments.

- `--no_frontend`: If this flag is set, the front-end is not included in-the-loop and fake input is used instead. The fake input comes from a text file, which should be specified with this flag. For example, `./main --no_fpga --print_request --no_frontend test_matrices.txt`. The program will terminate after a single calculation.

- `--no_fpga`: If this flag is set, the FPGA is not included in-the-loop and fake output is used instead.

- `--conv_mode`: Use in conjunction with `--no_frontend`. If this flag is set, the fake input matrices will be used for convolution rather than matmul.

- `--print_request`: If this flag is set, the incoming requests from the frontend will be printed for debugging purposes.

For example, to run the program with no FPGA connected, you would use:

`./main --no_fpga`

To run the program to perform convolution with no FPGA or frontend connected, you would use:
`./main --no_frontend convolution_files/test_edge_detect.txt --no_fpga --conv_mode`
