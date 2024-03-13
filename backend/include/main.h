#ifndef MAIN_H
#define MAIN_H
// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include "convolution.h"
#include "matmul.h"
#include "nlohmann/json.hpp"
#include <chrono>
#include <cstdint>

// *********************************************************************
// ||                           GLOBAL VARS                           ||
// *********************************************************************
extern bool FPGA_IN_LOOP;
extern bool FRONTEND_IN_LOOP;
extern bool PRINT_REQUEST;
extern bool CALIBRATION_MODE;

extern float calibration_factor_even;
extern float calibration_factor_odd;

#endif
