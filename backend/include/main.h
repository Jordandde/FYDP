#ifndef MAIN_H
#define MAIN_H
// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include "convolution.h"
#include "http_stuff.hpp"
#include "main.h"
#include "matmul.h"
#include "matrix.hpp"
#include "nlohmann/json.hpp"
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>

// *********************************************************************
// ||                           GLOBAL VARS                           ||
// *********************************************************************
extern bool FPGA_IN_LOOP;
extern bool FRONTEND_IN_LOOP;
extern bool PRINT_REQUEST;
extern bool CALIBRATION_MODE;

extern float calibration_factor_even;
extern float calibration_factor_odd;

// *********************************************************************
// ||                             DEFINES                             ||
// *********************************************************************
#define POST_PORT 0xB00B
#define FPGA_PAYLOAD_DIMS_SIZE 4 // num_rows1, num_cols1, num_rows2, num_cols2
#define FPGA_ADDR "192.168.1.10"
#define FPGA_PORT "5001"

#define MAX_DIMS 64 * 64
#define REQUEST_OVERHEAD 1024                                        // For the headers and stuff that come with the request
#define NUM_DIGITS_MAX 4                                             // Max number of digits for each matrix entry
#define RECV_BUF_SIZE (MAX_DIMS * NUM_DIGITS_MAX + REQUEST_OVERHEAD) // Buffer needs to be big enough to receive the entire request

#define FIXED_POINT_SCALING_FACTOR 1000
#define ADC_CONVERSION_FACTOR 3125 / (FIXED_POINT_SCALING_FACTOR * FIXED_POINT_SCALING_FACTOR) // ((10.24 * 10 * Nout) / 2^15) * (2^24)/(4.096^2)

#define CONV_FIXED_POINT_SCALING_FACTOR 10
#define CONV_ADC_CONVERSION_FACTOR 3125 / (CONV_FIXED_POINT_SCALING_FACTOR * CONV_FIXED_POINT_SCALING_FACTOR) // ((10.24 * 10 * Nout) / 2^15) * (2^24)/(4.096^2)
#define KERNEL_SCALING_FACTOR 10
#define KERNEL_SIZE 9

#endif
