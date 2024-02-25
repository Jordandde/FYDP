// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include <iostream>
#include <boost/program_options.hpp>
#include <cstdint>
#include "include/nlohmann/json.hpp"
#include "matrix.hpp"
#include "http_stuff.hpp"
#include <chrono>


// *********************************************************************
// ||                             DEFINES                             ||
// *********************************************************************
#define POST_PORT 0xB00B
#define FPGA_PAYLOAD_DIMS_SIZE 4 // num_rows1, num_cols1, num_rows2, num_cols2
#define FPGA_ADDR "192.168.1.10"
#define FPGA_PORT "5001"

#define MAX_DIMS 64*64
#define REQUEST_OVERHEAD 1024 // For the headers and stuff that come with the request
#define NUM_DIGITS_MAX 4 // Max number of digits for each matrix entry
#define RECV_BUF_SIZE (MAX_DIMS * NUM_DIGITS_MAX + REQUEST_OVERHEAD) // Buffer needs to be big enough to receive the entire request

namespace po = boost::program_options;

// *********************************************************************
// ||                           GLOBAL VARS                           ||
// *********************************************************************
bool FPGA_IN_LOOP = true;
bool FRONTEND_IN_LOOP = true;
bool PRINT_REQUEST = false;

int* create_out_fpga_payload(Matrices &matrices, const size_t out_fpga_payload_size)
{
        int* out_fpga_payload = (int*)malloc(out_fpga_payload_size * sizeof(int)); // Using int sizes for now. Can use smaller sizes if we think it's worth it & after we come up with size constraints
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

void handle_request(const std::string &request, ip::tcp::socket &frontend_socket, ip::tcp::socket &fpga_socket)
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

        std::cout << "Matrices received successfully!" << std::endl;

        if (in_matrices_payload.size() != 2)
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
            (void) bytes_transferred; // maybe do something with this idk
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
        const std::chrono::duration<double> diff = now - timer;
        std::cout << "PROFILER - Request to result: " << diff.count() << " seconds." << std::endl;
        // Print result matrix for debugging
        std::cout << "Result Matrix:" << std::endl;
        std::cout << matrices.result_matrix.to_string() << std::endl;

        // Send response with result matrix back to the front-end
        std::string response = matrices.result_matrix.to_string();
        if (FRONTEND_IN_LOOP)
        {
            send_good_response(frontend_socket, response);
        }
    }
    catch (const std::exception &e)
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
                size_t bytes_transferred;
                char buffer[RECV_BUF_SIZE];
                bytes_transferred = frontend_socket.read_some(boost::asio::buffer(buffer, RECV_BUF_SIZE), error);
                request.append(buffer, buffer + bytes_transferred);

                if (error == boost::asio::error::eof)
                {
                    // Connection closed cleanly by peer
                    std::cout << "Connection closed by peer." << std::endl;
                }
                else if (error)
                {
                    // Some other error
                    throw boost::system::system_error(error); // Rethrow the error
                }
                else
                {
                    // Trim the request string to actual size
                    request.resize(bytes_transferred);

                    // Process the request
                    handle_request(request, frontend_socket, fpga_socket);
                }
            }
            else
            {
                // Process a dummy request
                handle_request(std::string(DUMMY_FRONTEND_INPUT), frontend_socket, fpga_socket);
            }
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("no_frontend", "whether or not front-end is in the loop (fake input if this flag is true)")
        ("no_fpga", "whether or not FPGA is in the loop (fake output if this flag is true)")
        ("print_request", "print the full incoming requests from the frontend (for debugging)")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }
    
    if (vm.count("no_frontend")) {
        std::cout << "No front-end in the loop " << std::endl;
        FRONTEND_IN_LOOP = false;
    } else {
        std::cout << "Front-end in the loop" << std::endl;
        FRONTEND_IN_LOOP = true;
    }

    if (vm.count("no_fpga")) {
        std::cout << "No FPGA in the loop " << std::endl;
        FPGA_IN_LOOP = false;
    } else {
        std::cout << "FPGA in the loop" << std::endl;
        FPGA_IN_LOOP = true;
    }

    if (vm.count("print_request")) {
        PRINT_REQUEST = true;
    } else {
        PRINT_REQUEST = false;
    }

    start_server();
    return 0;
}
