// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include "matmul.h"

// *********************************************************************
// ||                             DEFINES                             ||
// *********************************************************************

// *********************************************************************
// ||                           GLOBAL VARS                           ||
// *********************************************************************
extern bool FPGA_IN_LOOP;
extern bool FRONTEND_IN_LOOP;
extern bool CALIBRATION_MODE;

extern float calibration_factor_even;
extern float calibration_factor_odd;

// *********************************************************************
// ||                           FUNCTIONS                             ||
// *********************************************************************
int* create_out_fpga_payload(Matrices& matrices, const size_t out_fpga_payload_size)
{
    int* out_fpga_payload =
        (int*)malloc(out_fpga_payload_size * sizeof(int)); // Using int sizes for now. Can use smaller sizes if we think it's worth it & after we come up with size constraints
    int pos = 0;

    // Copy in dimensions of input matrix 1
    out_fpga_payload[pos++] = matrices.input_matrix_1.get_num_rows();
    out_fpga_payload[pos++] = matrices.input_matrix_1.get_num_cols();

    // Copy in input matrix 1
    for (int i = 0; i < matrices.input_matrix_1.get_num_rows(); i++)
    {
        for (int j = 0; j < matrices.input_matrix_1.get_num_cols(); j++)
        {
            out_fpga_payload[pos++] = static_cast<int>(FIXED_POINT_SCALING_FACTOR * matrices.input_matrix_1[i][j]);
        }
    }

    // Copy in dimensions of input matrix 2 (transposed)
    out_fpga_payload[pos++] = matrices.input_matrix_2.get_num_cols();
    out_fpga_payload[pos++] = matrices.input_matrix_2.get_num_rows();

    // Copy in input matrix 2 (transposed)
    for (int j = 0; j < matrices.input_matrix_2.get_num_cols(); j++)
    {
        for (int i = 0; i < matrices.input_matrix_2.get_num_rows(); i++)
        {
            out_fpga_payload[pos++] = static_cast<int>(FIXED_POINT_SCALING_FACTOR * matrices.input_matrix_2[i][j]);
        }
    }

    if (pos != out_fpga_payload_size)
    {
        std::cout << "Error: pos = " << pos << ", out_fpga_payload_size = " << out_fpga_payload_size << std::endl;
    }

    // Print FPGA payload for debugging
    std::cout << "Out FPGA Payload:" << std::endl;
    for (int i = 0; i < out_fpga_payload_size; i++)
    {
        std::cout << out_fpga_payload[i] << " ";
    }
    std::cout << std::endl;

    return out_fpga_payload;
}

void handle_matmul_request(ip::tcp::socket& frontend_socket, ip::tcp::socket& fpga_socket, Matrices matrices)
{
    const auto timer = std::chrono::high_resolution_clock::now();
    // Print input matrices as sanity check
    std::cout << "Input Matrix 1:" << std::endl;
    std::cout << matrices.input_matrix_1.to_string() << std::endl;

    std::cout << "Input Matrix 2:" << std::endl;
    std::cout << matrices.input_matrix_2.to_string() << std::endl;

    // CONVERT FLOATS TO FPGA/HARDWARE FRIENDLY FORMAT, PROBABLY INTS??
    // Let's just start with ints for now. Floats can be handled with fixed precision by mulitplying by factor(s) of 10 and scaling the result accordingly
    size_t out_fpga_payload_size = (FPGA_PAYLOAD_DIMS_SIZE + matrices.input_matrix_1.get_num_rows() * matrices.input_matrix_1.get_num_cols() +
                                    matrices.input_matrix_2.get_num_rows() * matrices.input_matrix_2.get_num_cols());

    int* out_fpga_payload = create_out_fpga_payload(matrices, out_fpga_payload_size);

    if (FPGA_IN_LOOP)
    {
        // Send the payload to the FPGA
        std::cout << "Sending to FPGA..." << std::endl;
        boost::asio::write(fpga_socket, boost::asio::buffer((void*)out_fpga_payload, out_fpga_payload_size * sizeof(int)));
        std::cout << "Sent to FPGA..." << std::endl;
    }

    free(out_fpga_payload);

    // Receive the result from the FPGA
    size_t in_fpga_payload_size = matrices.result_matrix.get_num_rows() * matrices.result_matrix.get_num_cols();
    int* in_fpga_payload = (int*)malloc(in_fpga_payload_size * sizeof(int));

    std::cout << "in_fpga_payload_size = " << in_fpga_payload_size << std::endl;

    if (FPGA_IN_LOOP)
    {
        // Send matrices to FPGA for calculation
        std::cout << "Waiting for response from FPGA..." << std::endl;
        size_t bytes_transferred = fpga_socket.read_some(boost::asio::buffer((void*)in_fpga_payload, in_fpga_payload_size * sizeof(int)));
        (void)bytes_transferred; // maybe do something with this idk
        std::cout << "Received from FPGA..." << std::endl;
        // Print result from FPGA
        std::cout << "In FPGA Payload:" << std::endl;
        for (int i = 0; i < in_fpga_payload_size; i++)
        {
            std::cout << in_fpga_payload[i] << " ";
        }
        std::cout << std::endl;

        // Update result matrix with the result from the FPGA
        int pos = 0;
        for (int i = 0; i < matrices.result_matrix.get_num_rows(); i++)
        {
            for (int j = 0; j < matrices.result_matrix.get_num_cols(); j++)
            {
                // SCALE RESULTS "12-bit dac with 4.096 reference voltage -> analog switch with 15ohms of resistance
                // -> analog multipler that divides the result by a factor of 10 -> 16-bit adc with 4.096 reference voltage"
                if (i % 2)
                {
                    matrices.result_matrix[i][j] = calibration_factor_odd * ADC_CONVERSION_FACTOR * in_fpga_payload[pos++];
                }
                else
                {
                    matrices.result_matrix[i][j] = calibration_factor_even * ADC_CONVERSION_FACTOR * in_fpga_payload[pos++];
                }
            }
        }
    }
    else
    {
        // Multiply matrices 1 and 2 and store the result in the result matrix
        matrices.result_matrix = matrices.input_matrix_1 * matrices.input_matrix_2;
    }

    free(in_fpga_payload);

    if (CALIBRATION_MODE)
    {
        Matrix correct_matrix = matrices.input_matrix_1 * matrices.input_matrix_2;
        // Compare each element of correct matrix and result matrix to determine calibration factor
        float total_error_even = 0;
        float total_error_odd = 0;
        for (int i = 0; i < matrices.result_matrix.get_num_rows(); i++)
        {
            for (int j = 0; j < matrices.result_matrix.get_num_cols(); j++)
            {
                float error = correct_matrix[i][j] / matrices.result_matrix[i][j];
                if (i % 2)
                {
                    total_error_odd += error;
                }
                else
                {
                    total_error_even += error;
                }
                std::cout << "error = " << error << std::endl;
            }
        }
        calibration_factor_odd = std::clamp((double)2.0 * total_error_odd / (matrices.result_matrix.get_num_rows() * matrices.result_matrix.get_num_cols()), 0.95, 1.05);
        calibration_factor_even = std::clamp((double)2.0 * total_error_even / (matrices.result_matrix.get_num_rows() * matrices.result_matrix.get_num_cols()), 0.95, 1.05);
        // calibration_factor = total_error / (matrices.result_matrix.get_num_rows() * matrices.result_matrix.get_num_cols());
        std::cout << "Set even calibration factor to " << calibration_factor_even << std::endl;
        std::cout << "Set odd calibration factor to " << calibration_factor_odd << std::endl;
        CALIBRATION_MODE = false;
    }

    const auto now = std::chrono::high_resolution_clock::now();
    long long diff_us = std::chrono::duration_cast<std::chrono::microseconds>(now - timer).count();
    std::cout << "PROFILER - Request to result: " << diff_us << " microseconds." << std::endl;
    // Print result matrix for debugging
    std::cout << "Result Matrix:" << std::endl;
    std::cout << matrices.result_matrix.to_string() << std::endl;

    // Send response with result matrix back to the front-end
    std::string response = matrices.result_matrix.to_string_with_precision(2);
    if (FRONTEND_IN_LOOP)
    {
        send_good_response(frontend_socket, response);
    }
    else
    {
        // Print expected result matrix (computed locally)
        std::cout << "Expected Result Matrix:" << std::endl;
        std::cout << (matrices.input_matrix_1 * matrices.input_matrix_2).to_string() << std::endl;

        matrices.result_matrix -= (matrices.input_matrix_1 * matrices.input_matrix_2);
        std::cout << "Error Matrix:" << std::endl;
        std::cout << matrices.result_matrix.to_string() << std::endl;

        exit(0);
    }
}
