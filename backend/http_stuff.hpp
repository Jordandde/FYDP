// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include <boost/asio.hpp>

// *********************************************************************
// ||                             DEFINES                             ||
// *********************************************************************
// For use in lieu of input from frontend
#define DUMMY_FRONTEND_INPUT                                                                                                                                                       \
    "POST /matrices "                                                                                                                                                              \
    "HTTP/"                                                                                                                                                                        \
    "1.1\n{\"matrices\":[[[\"1\",\"4\",\"3\",\"4\"],[\"1\",\"2\",\"4\",\"4\"],[\"1\",\"2\",\"3\",\"4\"],[\"1\",\"2\",\"3\",\"4\"]],[[\"1\",\"1\",\"3\",\"1\"],[\"1\",\"0\",\"1\"," \
    "\"2\"],[\"1\",\"2\",\"3\",\"3\"],[\"1\",\"2\",\"0\",\"1\"]]]}"

using namespace boost::asio;

// *********************************************************************
// ||                            FUNCTIONS                            ||
// *********************************************************************
void handle_options_request(ip::tcp::socket& socket)
{
    // Send CORS headers for preflight request
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: DELETE, POST, GET, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\n";
    response += "Content-Length: 0\r\n\r\n";
    boost::asio::write(socket, boost::asio::buffer(response));
}

void send_good_response(ip::tcp::socket& socket, const std::string& response)
{
    boost::asio::streambuf response_buf;
    std::ostream response_stream(&response_buf);
    response_stream << "HTTP/1.1 200 OK\r\n";
    response_stream << "Content-Type: text/plain\r\n";
    response_stream << "Access-Control-Allow-Origin: *\r\n"; // Enable CORS for all origins
    response_stream << "Content-Length: " << response.size() << "\r\n\r\n";
    response_stream << response;
    boost::asio::write(socket, response_buf);
}

void send_bad_response(ip::tcp::socket& socket, const std::string& response)
{
    boost::asio::streambuf response_buf;
    std::ostream response_stream(&response_buf);
    response_stream << "HTTP/1.1 400 Bad Request\r\n";
    response_stream << "Content-Type: text/plain\r\n";
    response_stream << "Access-Control-Allow-Origin: *\r\n"; // Enable CORS for all origins
    response_stream << "Content-Length: " << response.size() << "\r\n\r\n";
    response_stream << response;
    boost::asio::write(socket, response_buf);
}

void send_method_not_allowed_response(ip::tcp::socket& socket)
{
    std::string response = "HTTP/1.1 405 Method Not Allowed\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Access-Control-Allow-Origin: *\r\n"; // Enable CORS for all origins
    response += "Content-Length: 0\r\n\r\n";
    boost::asio::write(socket, boost::asio::buffer(response));
}
