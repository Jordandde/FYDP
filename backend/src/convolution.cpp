// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include "convolution.h"

// *********************************************************************
// ||                             DEFINES                             ||
// *********************************************************************

// *********************************************************************
// ||                           FUNCTIONS                             ||
// *********************************************************************
int* create_out_fpga_convolution_payload(Matrix kernel, Matrix image_data, const size_t out_fpga_payload_size)
{
    int* out_fpga_payload =
        (int*)malloc(out_fpga_payload_size * sizeof(int)); // Using int sizes for now. Can use smaller sizes if we think it's worth it & after we come up with size constraints
    int pos = 0;

    // Copy in dimensions of input matrix 1
    out_fpga_payload[pos++] = kernel.get_num_rows();
    out_fpga_payload[pos++] = kernel.get_num_cols();

    // Copy in input matrix 1
    for (int i = 0; i < kernel.get_num_rows(); i++)
    {
        for (int j = 0; j < kernel.get_num_cols(); j++)
        {
            out_fpga_payload[pos++] = static_cast<int>(CONV_FIXED_POINT_SCALING_FACTOR * kernel[i][j]);
        }
    }

    // Copy in dimensions of input matrix 2 (transposed)
    out_fpga_payload[pos++] = image_data.get_num_cols();
    out_fpga_payload[pos++] = image_data.get_num_rows();

    // Copy in input matrix 2 (transposed)
    for (int j = 0; j < image_data.get_num_cols(); j++)
    {
        for (int i = 0; i < image_data.get_num_rows(); i++)
        {
            out_fpga_payload[pos++] = static_cast<int>(CONV_FIXED_POINT_SCALING_FACTOR * image_data[i][j]);
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

void handle_conv_request(ip::tcp::socket& frontend_socket, ip::tcp::socket& fpga_socket, Matrix kernel, Matrix image)
{
    const auto timer = std::chrono::high_resolution_clock::now();
    // Matrices matrices(in_matrices_payload[0], in_matrices_payload[1]); // First matrix is the image, second matrix is the kernel
    // Matrix kernel(static_cast<std::vector<std::vector<std::string>>>(in_matrices_payload[0]));
    // Matrix image(static_cast<std::vector<std::vector<std::string>>>(in_matrices_payload[1]));

    // Print input matrices as sanity check
    // std::cout << "Kernel: " << std::endl;
    // std::cout << kernel.to_string() << std::endl;

    // std::cout << "Image: " << std::endl;
    // std::cout << image.to_string() << std::endl;

    Matrix result_matrix(image.get_num_rows(), image.get_num_cols());

    image.pad();

    // Print padded image for debugging
    // std::cout << "Padded Image: " << std::endl;
    // std::cout << image.to_string() << std::endl;

    // Flatten kernel matrix
    Matrix flattened_kernel(1, 9);

    int pos = 0;
    for (int i = 0; i < kernel.get_num_rows(); i++)
    {
        for (int j = 0; j < kernel.get_num_cols(); j++)
        {
            flattened_kernel[0][pos] = kernel[i][j];
            pos++;
        }
    }

    // Scale kernel to reduce error due to noise
    flattened_kernel.scale(KERNEL_SCALING_FACTOR);

    // Print flattened kernel for debugging
    std::cout << "Flattened Kernel: " << std::endl;
    std::cout << flattened_kernel.to_string() << std::endl;

    // Iterate through image matrix one row at a time, and multiply by the flattened nine surrounding pixels for each to perform convolution using multiplication. The input
    // matrices should be 1x9 and 9xn respectively, for an output matrix of 1xn.
    for (int i = 1; i < image.get_num_rows() - 1; i++)
    {
        std::vector<std::vector<float>> image_data;

        for (int j = 1; j < image.get_num_cols() - 1; j++)
        {
            std::vector<float> submatrix_data;
            // Pack surrounding pixels into a 1x9 matrix
            for (int k = -1; k <= 1; k++)
            {
                for (int l = -1; l <= 1; l++)
                {
                    submatrix_data.push_back(image[i + k][j + l]);
                }
            }
            image_data.push_back(submatrix_data);
        }

        // Print image_data
        // std::cout << "Image Data:" << std::endl;
        // for (int i = 0; i < image_data.size(); i++)
        // {
        //     for (int j = 0; j < image_data[i].size(); j++)
        //     {
        //         std::cout << image_data[i][j] << " ";
        //     }
        //     std::cout << std::endl;
        // }

        // Send image_data for this row alongside the kernel to FPGA for computation
        Matrix image_data_matrix(image_data, true);

        // Print image_data_matrix
        // std::cout << "Image Data Matrix:" << std::endl;
        // std::cout << image_data_matrix.to_string() << std::endl;

        size_t out_fpga_payload_size = (FPGA_PAYLOAD_DIMS_SIZE + KERNEL_SIZE + (KERNEL_SIZE * image_data_matrix.get_num_cols()));
        int* out_fpga_payload = create_out_fpga_convolution_payload(flattened_kernel, image_data_matrix, out_fpga_payload_size);

        if (FPGA_IN_LOOP)
        {
            // Send the payload to the FPGA
            std::cout << "Sending to FPGA..." << std::endl;
            boost::asio::write(fpga_socket, boost::asio::buffer((void*)out_fpga_payload, out_fpga_payload_size * sizeof(int)));
            std::cout << "Sent to FPGA..." << std::endl;
        }

        free(out_fpga_payload);

        if (FPGA_IN_LOOP)
        {
            // Receive the result from the FPGA
            size_t in_fpga_payload_size = image_data_matrix.get_num_cols(); // Result should be all of the pixels in the result image flattened into one row
            int* in_fpga_payload = (int*)malloc(in_fpga_payload_size * sizeof(int));

            std::cout << "in_fpga_payload_size = " << in_fpga_payload_size << std::endl;

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
            for (int col = 0; col < result_matrix.get_num_cols(); col++)
            {
                result_matrix[i - 1][col] = CONV_ADC_CONVERSION_FACTOR * in_fpga_payload[pos++];
            }
            free(in_fpga_payload);
        }
        else
        {
            // Multiply matrices 1 and 2 and store the result in the result matrix
            Matrix result_row(flattened_kernel * image_data_matrix);
            // Print result row for debugging
            // std::cout << "Result Row:" << std::endl;
            // std::cout << result_row.to_string() << std::endl;
            result_matrix[i - 1] = result_row[0]; // offset by 1 to account for matrix padding
        }
    }

    // Find minimum value in result matrix
    float min_val = 0;
    float max_val = 0;
    for (int i = 0; i < result_matrix.get_num_rows(); i++)
    {
        for (int j = 0; j < result_matrix.get_num_cols(); j++)
        {
            if (result_matrix[i][j] < min_val)
            {
                min_val = result_matrix[i][j];
            }
            if (result_matrix[i][j] > max_val)
            {
                max_val = result_matrix[i][j];
            }
        }
    }

    // Min-max normalize the result matrix to [0,255]
    for (int i = 0; i < result_matrix.get_num_rows(); i++)
    {
        for (int j = 0; j < result_matrix.get_num_cols(); j++)
        {
            result_matrix[i][j] = 255.0F * (result_matrix[i][j] - min_val) / (max_val - min_val);
        }
    }

    const auto now = std::chrono::high_resolution_clock::now();
    long long diff_us = std::chrono::duration_cast<std::chrono::microseconds>(now - timer).count();
    std::cout << "PROFILER - Request to result: " << diff_us << " microseconds." << std::endl;
    // Print result matrix for debugging
    // std::cout << "Result Matrix:" << std::endl;
    // std::cout << result_matrix.to_string_with_precision(0) << std::endl;
    std::cout << "Got result matrix of size " << result_matrix.get_num_rows() << "x" << result_matrix.get_num_cols() << std::endl;

    // Write result matrix to a file
    std::ofstream result_file("convolution_files/result_matrix.txt");
    if (result_file.is_open())
    {
        result_file << result_matrix.to_string_with_precision(0);
        result_file.close();
    }
    else
    {
        std::cout << "Unable to open file for writing result matrix." << std::endl;
    }

    // Send response with result matrix back to the front-end
    std::string response = result_matrix.to_string_with_precision(0);
    if (FRONTEND_IN_LOOP)
    {
        send_good_response(frontend_socket, response);
    }
    else
    {
        // // Print expected result matrix (computed locally)
        // std::cout << "Expected Result Matrix:" << std::endl;
        // std::cout << (flattened_kernel * matrices.input_matrix_2).to_string() << std::endl;

        // matrices.result_matrix -= (flattened_kernel * matrices.input_matrix_2);
        // std::cout << "Error Matrix:" << std::endl;
        // std::cout << matrices.result_matrix.to_string() << std::endl;

        exit(0);
    }
}
