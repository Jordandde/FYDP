#include <iostream>
#include <boost/asio.hpp>
#include "include/nlohmann/json.hpp"

using namespace boost::asio;

#define POST_PORT 0xB00B

// Define the structure for storing matrices
class Matrices {
    public:
        // TODO: Make these floats
        std::vector<std::vector<std::string>> input_matrix_1;
        std::vector<std::vector<std::string>> input_matrix_2;
        std::vector<std::vector<std::string>> result_matrix;

        Matrices(std::vector<std::vector<std::string>> input1, std::vector<std::vector<std::string>> input2)
        {
            input_matrix_1 = input1;
            input_matrix_2 = input2;
            int num_rows_output = input1.size();
            int num_cols_output = input2[0].size();
            result_matrix.resize(num_rows_output, std::vector<std::string>(num_cols_output, "0"));
        }
};

void handle_options_request(ip::tcp::socket& socket) {
    // Send CORS headers for preflight request
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: DELETE, POST, GET, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\n";
    response += "Content-Length: 0\r\n\r\n";
    boost::asio::write(socket, boost::asio::buffer(response));
}

void handle_request(const std::string& request, ip::tcp::socket& socket) {
    try {

        std::cout << "Request received: " << request << std::endl;
        // Handle OPTIONS request
        if (request.substr(0, 7) == "OPTIONS") {
            handle_options_request(socket);
            return;
        }

        else if (request.substr(0, 4) != "POST") {
            // Send a 405 Method Not Allowed response
            std::string response = "HTTP/1.1 405 Method Not Allowed\r\n";
            response += "Content-Type: text/plain\r\n";
            response += "Access-Control-Allow-Origin: *\r\n"; // Enable CORS for all origins
            response += "Content-Length: 0\r\n\r\n";
            boost::asio::write(socket, boost::asio::buffer(response));
            return;
        }

        if (request.empty()) {
            // std::cout << "Empty request received." << std::endl;
            throw std::invalid_argument("Empty request received.");
            std::cout << "Empty request received." << std::endl;
            return;
        }

        // Find the position of the first '{' character after the headers
        size_t json_start_pos = request.find("{");
        if (json_start_pos == std::string::npos) {
            throw std::invalid_argument("JSON data not found in request.");
        }

        // Extract the JSON payload from the request
        std::string json_payload = request.substr(json_start_pos);

        // Parse the JSON payload into the Matrices struct
        auto matrices_payload = nlohmann::json::parse(json_payload)["matrices"];

        std::cout << "Matrices received successfully!" << std::endl;

        Matrices matrices(matrices_payload[0], matrices_payload[1]);

        // Print input matrices as sanity check
        std::cout << "Input Matrix 1:" << std::endl;
        for (const auto& row : matrices.input_matrix_1) {
            for (const auto& cell : row) {
                std::cout << cell << " ";
            }
            std::cout << std::endl;
        }
        
        std::cout << "Input Matrix 2:" << std::endl;
        for (const auto& row : matrices.input_matrix_2) {
            for (const auto& cell : row) {
                std::cout << cell << " ";
            }
            std::cout << std::endl;
        }

        // TALK WITH FPGA AND GET RESULTS

        // SCALE RESULTS "12-bit dac with 4.096 reference voltage -> analog switch with 15ohms of resistance 
        // -> analog multipler that divides the result by a factor of 10 -> 16-bit adc with 4.096 reference voltage"

        // UPDATE RESULT MATRIX

        // Print result matrix for debugging
        std::cout << "Result Matrix:" << std::endl;
        for (const auto& row : matrices.result_matrix) {
            for (const auto& cell : row) {
                std::cout << cell << " ";
            }
            std::cout << std::endl;
        }

        // Send a response back to the frontend with CORS headers
        std::string response = "Matrices received successfully!";
        boost::asio::streambuf response_buf;
        std::ostream response_stream(&response_buf);
        response_stream << "HTTP/1.1 200 OK\r\n";
        response_stream << "Content-Type: text/plain\r\n";
        response_stream << "Access-Control-Allow-Origin: *\r\n"; // Enable CORS for all origins
        response_stream << "Content-Length: " << response.size() << "\r\n\r\n";
        response_stream << response;
        boost::asio::write(socket, response_buf);
    } catch (const std::exception& e) {
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

void start_server() {
    try {
        // Create io_context
        io_context io;

        // Create an acceptor to listen for incoming connections
        ip::tcp::acceptor acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), POST_PORT));

        std::cout << "Server started. Waiting for incoming connections..." << std::endl;

        // Start accepting connections
        while (true) {
            // Create a socket for the incoming connection
            ip::tcp::socket socket(io);

            // Wait for a client to connect
            acceptor.accept(socket);

            // Read request data from the client
            std::string request;
            boost::system::error_code error;
            size_t bytes_transferred;
            do {
                char buffer[1024]; // Adjust buffer size as needed
                bytes_transferred = socket.read_some(boost::asio::buffer(buffer), error);
                request.append(buffer, buffer + bytes_transferred);
            } while (0);

            if (error == boost::asio::error::eof) {
                // Connection closed cleanly by peer
                std::cout << "Connection closed by peer." << std::endl;
            } else if (error) {
                // Some other error
                throw boost::system::system_error(error); // Rethrow the error
            } else {
                // Trim the request string to actual size
                request.resize(bytes_transferred);

                // std::cout << "Transferred " << bytes_transferred << " bytes." << std::endl;
                // std::cout << "Request received: " << request << std::endl;

                // Process the request
                handle_request(request, socket);
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {
    start_server();
    return 0;
}
