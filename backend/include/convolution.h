#ifndef CONVOLUTION_H
#define CONVOLUTION_H
// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include "constants.h"
#include "http_stuff.hpp"
#include "matrix.hpp"
#include <fstream>
#include <iostream>

// *********************************************************************
// ||                           FUNCTIONS                             ||
// *********************************************************************
int* create_out_fpga_convolution_payload(Matrix kernel, Matrix image_data, const size_t out_fpga_payload_size);

void handle_conv_request(ip::tcp::socket& frontend_socket, ip::tcp::socket& fpga_socket, Matrix kernel, Matrix image);

#endif