// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include "http_stuff.hpp"
#include "include/nlohmann/json.hpp"
#include "matrix.hpp"
#include <boost/program_options.hpp>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>

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

namespace po = boost::program_options;

// *********************************************************************
// ||                           GLOBAL VARS                           ||
// *********************************************************************
bool FPGA_IN_LOOP = true;
bool FRONTEND_IN_LOOP = true;
bool PRINT_REQUEST = false;

std::vector<std::vector<std::string>> fake_input1;
std::vector<std::vector<std::string>> fake_input2;

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
            out_fpga_payload[pos++] = static_cast<int>(matrices.input_matrix_1[i][j]);
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
            out_fpga_payload[pos++] = static_cast<int>(matrices.input_matrix_2[i][j]);
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
        std::cout << out_fpga_payload[i];
    }
    std::cout << std::endl;

    return out_fpga_payload;
}

void handle_request(const std::string& request, ip::tcp::socket& frontend_socket, ip::tcp::socket& fpga_socket)
{
    try
    {
        if (PRINT_REQUEST)
        {
            std::cout << "Request received: " << request << std::endl;
        }

        if (request.empty())
        {
            std::cout << "Empty request received." << std::endl;
            // throw std::invalid_argument("Empty request received.");
            return;
        }
        else if (request.substr(0, 7) == "OPTIONS")
        {
            handle_options_request(frontend_socket);
            return;
        }
        else if (request.substr(0, 4) != "POST")
        {
            send_method_not_allowed_response(frontend_socket);
            return;
        }
        const auto timer = std::chrono::high_resolution_clock::now();
        // Find the position of the first '{' character after the headers
        size_t json_start_pos = request.find("{");
        if (json_start_pos == std::string::npos)
        {
            throw std::invalid_argument("JSON data not found in request.");
        }

        // Extract the JSON payload from the request and parse it into the Matrices struct
        std::string in_json_payload = request.substr(json_start_pos);
        auto in_matrices_payload = nlohmann::json::parse(in_json_payload)["matrices"];
        auto calibration_matrix  = nlohmann::json::parse(in_json_payload)["calibrationMatrix"];

        std::cout << "Matrices received successfully!" << std::endl;
        if (calibration_matrix.size() == 2) {
            //TODO: insert calibration logic here
            std::cout << "calibration mode"<<std::endl;
            in_matrices_payload = calibration_matrix;
        } else if (in_matrices_payload.size() != 2)
        {
            std::cout << "Expected 2 matrices, received " << in_matrices_payload.size() << " matrices." << std::endl;
            return;
        }

        Matrices matrices(in_matrices_payload[0], in_matrices_payload[1]);

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
                std::cout << in_fpga_payload[i];
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
                    matrices.result_matrix[i][j] = in_fpga_payload[pos++];
                }
            }
        }
        else
        {
            // Multiply matrices 1 and 2 and store the result in the result matrix
            matrices.result_matrix = matrices.input_matrix_1 * matrices.input_matrix_2;
        }
        const auto now = std::chrono::high_resolution_clock::now();
        long long diff_us = std::chrono::duration_cast<std::chrono::microseconds>(now - timer).count();
        std::cout << "PROFILER - Request to result: " << diff_us << " microseconds." << std::endl;
        // Print result matrix for debugging
        std::cout << "Result Matrix:" << std::endl;
        std::cout << matrices.result_matrix.to_string() << std::endl;

        // Send response with result matrix back to the front-end
        std::string response = matrices.result_matrix.to_string();
        if (FRONTEND_IN_LOOP)
        {
            send_good_response(frontend_socket, response);
        }
        else
        {
            exit(0);
        }
    }
    catch (const std::exception& e)
    {
        // Send an error response back to the front-end
        std::cout << "Error handling request: " << e.what() << std::endl;
        std::string response = "Error handling request: ";
        response += e.what();
        send_bad_response(frontend_socket, response);
    }
}

// Completely AI-cooked, I have no idea what's going on here but it looks alright to me and it works
void start_server()
{
    while (true)
    {
        try
        {
            // Create frontend socket and establish connection
            boost::asio::io_context io;
            ip::tcp::acceptor acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), POST_PORT));
            ip::tcp::socket frontend_socket(io);
            if (FRONTEND_IN_LOOP)
            {
                std::cout << "Waiting for client to connect..." << std::endl;
                acceptor.accept(frontend_socket);
                std::cout << "Client connected..." << std::endl;
            }
            else
            {
                std::cout << "Skipping connection to frontend..." << std::endl;
            }

            // Create FPGA socket and establish connection
            boost::asio::io_context io_context;
            ip::tcp::resolver resolver(io_context);
            ip::tcp::resolver::query query(FPGA_ADDR, FPGA_PORT);
            ip::tcp::resolver::results_type endpoints = resolver.resolve(query);
            ip::tcp::socket fpga_socket(io_context);
            if (FPGA_IN_LOOP)
            {
                std::cout << "Connecting to FPGA..." << std::endl;
                boost::asio::connect(fpga_socket, endpoints);
                std::cout << "Connected to FPGA..." << std::endl;
            }
            else
            {
                std::cout << "Skipping connection to FPGA..." << std::endl;
            }

            // Start handling incoming requests
            while (true)
            {
                // Read request data from the client
                std::string request;
                if (FRONTEND_IN_LOOP)
                {
                    boost::system::error_code error;
                    boost::asio::read_until(frontend_socket, boost::asio::dynamic_buffer(request), "}", error);

                    if (error == boost::asio::error::eof)
                    {
                        // Connection closed cleanly by peer
                        std::cout << "Connection closed by peer. Attempting to reconnect..." << std::endl;
                        // Sleep for a bit before attempting to reconnect
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        break;
                    }
                    else if (error)
                    {
                        // Some other error
                        throw boost::system::system_error(error); // Rethrow the error
                    }
                    else
                    {
                        // Remove anything that might've snuck in after the '}' character
                        std::size_t closing_pos = request.find_last_of('}');
                        if (closing_pos != std::string::npos) {
                            request.resize(closing_pos + 1);
                        }
                        // Process the request
                        handle_request(request, frontend_socket, fpga_socket);
                    }
                }
                else
                {
                    // Process a dummy request
                    handle_request(convert_txt_matrices_to_http_req(fake_input1, fake_input2), frontend_socket, fpga_socket);
                }
            }
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help", "produce help message")
        ("no_frontend", po::value<std::string>(), "whether or not front-end is in the loop (fake input if this flag is true)")
        ("no_fpga", "whether or not FPGA is in the loop (fake output if this flag is true)")
        ("print_request", "print the full incoming requests from the frontend (for debugging)");
    // clang-format on

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << "\n";
        return 1;
    }

    if (vm.count("no_frontend"))
    {
        std::cout << "No front-end in the loop " << std::endl;
        FRONTEND_IN_LOOP = false;

        std::string filename = vm["no_frontend"].as<std::string>();
        std::ifstream file(filename);
        if (!file)
        {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return 1;
        }

        std::vector<std::vector<std::string>>* current_matrix = &fake_input1;
        std::string line;
        while (std::getline(file, line))
        {
            // If the line is empty, switch to the second matrix
            if (line.empty())
            {
                current_matrix = &fake_input2;
                continue;
            }

            std::vector<std::string> row;
            std::istringstream iss(line);
            std::string value;
            while (std::getline(iss, value, ' ')) // Split the line into cells
            {
                row.push_back(value);
            }
            current_matrix->push_back(row);
        }
    }
    else
    {
        std::cout << "Front-end in the loop" << std::endl;
        FRONTEND_IN_LOOP = true;
    }

    if (vm.count("no_fpga"))
    {
        std::cout << "No FPGA in the loop " << std::endl;
        FPGA_IN_LOOP = false;
    }
    else
    {
        std::cout << "FPGA in the loop" << std::endl;
        FPGA_IN_LOOP = true;
    }

    if (vm.count("print_request"))
    {
        PRINT_REQUEST = true;
    }
    else
    {
        PRINT_REQUEST = false;
    }

    start_server();
    return 0;
}
