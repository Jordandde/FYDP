// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include <iostream>
#include <boost/asio.hpp>
#include <cstdint>
#include "include/nlohmann/json.hpp"
#include "matrix.hpp"

using namespace boost::asio;

// *********************************************************************
// ||                             DEFINES                             ||
// *********************************************************************
#define POST_PORT 0xB00B
#define FPGA_PAYLOAD_DIMS_SIZE 4 // num_rows1, num_cols1, num_rows2, num_cols2
#define FPGA_ADDR "192.168.1.10"
#define FPGA_PORT "5001"

// *********************************************************************
// ||                            FUNCTIONS                            ||
// *********************************************************************
// AI did this, I have no idea if this is legit -Pat
void handle_options_request(ip::tcp::socket &socket)
{
    // Send CORS headers for preflight request
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: DELETE, POST, GET, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\n";
    response += "Content-Length: 0\r\n\r\n";
    boost::asio::write(socket, boost::asio::buffer(response));
}

void handle_request(const std::string &request, ip::tcp::socket &socket)
{
    try
    {
        std::cout << "Request received: " << request << std::endl;
        // Handle OPTIONS request
        if (request.substr(0, 7) == "OPTIONS")
        {
            handle_options_request(socket);
            return;
        }

        else if (request.substr(0, 4) != "POST")
        {
            // Send a 405 Method Not Allowed response
            std::string response = "HTTP/1.1 405 Method Not Allowed\r\n";
            response += "Content-Type: text/plain\r\n";
            response += "Access-Control-Allow-Origin: *\r\n"; // Enable CORS for all origins
            response += "Content-Length: 0\r\n\r\n";
            boost::asio::write(socket, boost::asio::buffer(response));
            return;
        }

        if (request.empty())
        {
            // std::cout << "Empty request received." << std::endl;
            throw std::invalid_argument("Empty request received.");
            std::cout << "Empty request received." << std::endl;
            return;
        }

        // Find the position of the first '{' character after the headers
        size_t json_start_pos = request.find("{");
        if (json_start_pos == std::string::npos)
        {
            throw std::invalid_argument("JSON data not found in request.");
        }

        // Extract the JSON payload from the request
        std::string in_json_payload = request.substr(json_start_pos);

        // Parse the JSON payload into the Matrices struct
        auto in_matrices_payload = nlohmann::json::parse(in_json_payload)["matrices"];

        std::cout << "Matrices received successfully!" << std::endl;

        if (in_matrices_payload.size() != 2)
        {
            std::cout << "Expected 2 matrices, received " << in_matrices_payload.size() << " matrices." << std::endl;
        }

        Matrices matrices(in_matrices_payload[0], in_matrices_payload[1]);

        // Print input matrices as sanity check
        std::cout << "Input Matrix 1:" << std::endl;
        for (const auto &row : matrices.input_matrix_1)
        {
            for (const auto &cell : row)
            {
                std::cout << cell << " ";
            }
            std::cout << std::endl;
        }

        std::cout << "Input Matrix 2:" << std::endl;
        for (const auto &row : matrices.input_matrix_2)
        {
            for (const auto &cell : row)
            {
                std::cout << cell << " ";
            }
            std::cout << std::endl;
        }

        // CONVERT FLOATS TO FPGA/HARDWARE FRIENDLY FORMAT, PROBABLY INTS??
        // Let's just start with ints for now. Floats can be handled with fixed precision by mulitplying by factor(s) of 10 and scaling the result accordingly
        size_t out_fpga_payload_size = (FPGA_PAYLOAD_DIMS_SIZE + matrices.input_matrix_1.get_num_rows() * matrices.input_matrix_1.get_num_cols() + 
                                    matrices.input_matrix_2.get_num_rows() * matrices.input_matrix_2.get_num_cols());

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

        // Copy in dimensions of input matrix 2
        out_fpga_payload[pos++] = matrices.input_matrix_2.get_num_rows();
        out_fpga_payload[pos++] = matrices.input_matrix_2.get_num_cols();

        // Copy in input matrix 2
        for (int i = 0; i < matrices.input_matrix_2.get_num_rows(); i++)
        {
            for (int j = 0; j < matrices.input_matrix_2.get_num_cols(); j++)
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

        // TALK WITH FPGA AND GET RESULTS
        // Boilerplate from ChatGPT (does this socket connection stuff have to happen every time?? Or can it be put in main() and run once?)
        boost::asio::io_context io_context;

        // Resolve the server address and port
        ip::tcp::resolver resolver(io_context);
        ip::tcp::resolver::query query(FPGA_ADDR, FPGA_PORT);
        ip::tcp::resolver::results_type endpoints = resolver.resolve(query);

        // Establish a connection to the server
        ip::tcp::socket socket(io_context);
        std::cout << "Connecting to FPGA..." << std::endl;
        boost::asio::connect(socket, endpoints);
        std::cout << "Connected to FPGA..." << std::endl;
        
        // Send the payload to the FPGA
        std::cout << "Sending to FPGA..." << std::endl;
        boost::asio::write(socket, boost::asio::buffer((void*)out_fpga_payload, out_fpga_payload_size * sizeof(int)));
        std::cout << "Sent to FPGA..." << std::endl;

        free(out_fpga_payload);
        

        // Receive the result from the FPGA
        size_t in_fpga_payload_size = matrices.result_matrix.get_num_rows() * matrices.result_matrix.get_num_cols();

        int* in_fpga_payload = (int*)malloc(in_fpga_payload_size * sizeof(int));

        size_t bytes_transferred = socket.read_some(boost::asio::buffer((void*)in_fpga_payload, in_fpga_payload_size * sizeof(int)));
        // Print result from FPGA
        std::cout << "In FPGA Payload:" << std::endl;
        for (int i = 0; i < in_fpga_payload_size; i++)
        {
            std::cout << in_fpga_payload[i];
        }
        std::cout << std::endl;

        // SCALE RESULTS "12-bit dac with 4.096 reference voltage -> analog switch with 15ohms of resistance
        // -> analog multipler that divides the result by a factor of 10 -> 16-bit adc with 4.096 reference voltage"

        // UPDATE RESULT MATRIX
        // TODO: REMOVE THIS DUMMY OPERATION
        // Multiply matrices 1 and 2 and store the result in the result matrix
        // This is a leetcode question I think LOL
        for (int i = 0; i < matrices.input_matrix_1.get_num_cols(); i++)
        {
            for (int j = 0; j < matrices.input_matrix_2.get_num_rows(); j++)
            {
                for (int k = 0; k < matrices.input_matrix_2.get_num_cols(); k++)
                {
                    matrices.result_matrix[i][j] += matrices.input_matrix_1[i][k] * matrices.input_matrix_2[k][j];
                }
            }
        }

        // Print result matrix for debugging
        std::cout << "Result Matrix:" << std::endl;
        for (const auto &row : matrices.result_matrix)
        {
            for (const auto &cell : row)
            {
                printf("%.2f ", cell);
            }
            std::cout << std::endl;
        }

        // AI did these, I have no idea if this is legit -Pat
        // Send a response back to the frontend with CORS headers
        // std::string response = "Matrices received successfully!";
        std::string response = matrices.result_matrix.to_string();
        boost::asio::streambuf response_buf;
        std::ostream response_stream(&response_buf);
        response_stream << "HTTP/1.1 200 OK\r\n";
        response_stream << "Content-Type: text/plain\r\n";
        response_stream << "Access-Control-Allow-Origin: *\r\n"; // Enable CORS for all origins
        response_stream << "Content-Length: " << response.size() << "\r\n\r\n";
        response_stream << response;
        boost::asio::write(socket, response_buf);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error handling request: " << e.what() << std::endl;
        // Send an error response back to the frontend
        std::string response = "Error handling request: ";
        response += e.what();
        boost::asio::streambuf response_buf;
        std::ostream response_stream(&response_buf);
        response_stream << "HTTP/1.1 500 Internal Server Error\r\n";
        response_stream << "Content-Type: text/plain\r\n";
        response_stream << "Access-Control-Allow-Origin: *\r\n"; // Enable CORS for all origins
        response_stream << "Content-Length: " << response.size() << "\r\n\r\n";
        response_stream << response;
        boost::asio::write(socket, response_buf);
    }
}

// Completely AI-cooked, I have no idea what's going on here but it looks alright to me and it works
void start_server()
{
    try
    {
        // Create io_context
        io_context io;

        // Create an acceptor to listen for incoming connections
        ip::tcp::acceptor acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), POST_PORT));

        std::cout << "Server started. Waiting for incoming connections..." << std::endl;

        // Start accepting connections
        while (true)
        {
            // Create a socket for the incoming connection
            ip::tcp::socket socket(io);

            // Wait for a client to connect
            acceptor.accept(socket);

            // Read request data from the client
            std::string request;
            boost::system::error_code error;
            size_t bytes_transferred;
            char buffer[1024]; // Adjust buffer size as needed
            bytes_transferred = socket.read_some(boost::asio::buffer(buffer), error);
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
                handle_request(request, socket);
            }
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main()
{
    start_server();
    return 0;
}
