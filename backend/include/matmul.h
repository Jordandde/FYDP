#ifndef MATMUL_H
#define MATMUL_H
// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include "main.h"

// *********************************************************************
// ||                           FUNCTIONS                             ||
// *********************************************************************
int* create_out_fpga_payload(Matrices& matrices, const size_t out_fpga_payload_size);

void handle_matmul_request(ip::tcp::socket& frontend_socket, ip::tcp::socket& fpga_socket, Matrices matrices);

#endif
