/******************************************
*               Includes                  *
*******************************************/
#include <iostream>
#include <boost/asio.hpp>

/************************************************************************************************/
/******************************************* DEFINES *******************************************/
/************************************************************************************************/
#define FRONTEND_PORT 0xB00B


using namespace boost::asio;

/************************************************************************************************/
/******************************************* THE CODE *******************************************/
/************************************************************************************************/
void handle_request(const std::string& request, ip::tcp::socket& socket) {
    // Placeholder function for processing the request
    // In a real application, you would perform actual processing here
    std::string response = "Processed: " + request;

    std::cout << response << std::endl;

    // Send the response back to the frontend
    // boost::asio::write(socket, boost::asio::buffer(response));
}

void start_server() {
    try {
        // Create io_context
        io_context io;

        // Create an acceptor to listen for incoming connections
        ip::tcp::acceptor acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), FRONTEND_PORT));

        std::cout << "Server started. Waiting for incoming connections..." << std::endl;

        // Start accepting connections
        while (true) {
            // Create a socket for the incoming connection
            ip::tcp::socket socket(io);

            // Wait for a client to connect
            acceptor.accept(socket);

            // // Read request data from the client
            std::string request(1024, '\0');
            // size_t bytes_transferred = socket.read_some(boost::asio::buffer(request));

            // // Trim the request string to actual size
            // request.resize(bytes_transferred);

            // Process the request
            handle_request(request, socket);
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main() {
    start_server();
    return 0;
}
