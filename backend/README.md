# Getting started
The backend of our project uses {insert cpp or go} to pass matrices from the frontend to the TCP connected FPGA. 
This is done via json from the frontend to backend and an ethernet connection from the backend to the FPGA.

## Objective of the backend
The backend should do 4 things:
1. Start up in the background when the desktop app icon is clicked
2. listen for a json message, and then process the message when it is received
3. package the received data into a useable package that can be attached to the TCP call and sent to the FPGA
4. listen and wait for a response from the FPGA

## Precautions
1. Make sure to add a timeout while the backend is listening to the FPGA to avoid hanging the whole application
2. Add a calibration step as environment conditions introduce a (supposedly) linear error in multiplications


## Technology
The backend should consist of a simple http server. I propose that we use Go for this, as it is the main use of Go and debugging it is extremely easy. Alternatives include Python, CPP, Rust, and C.

Patrick: Noted. I'd prefer to use C++ due to familiarity

## Build Command
On Mac OS: 
`clang++ -std=c++17 -I /usr/local/Cellar/boost/1.84.0/include -L /usr/local/Cellar/boost/1.84.0/lib -lboost_system main.cpp -o main` 