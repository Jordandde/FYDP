#ifndef HTTP_STUFF_HPP
#define HTTP_STUFF_HPP
// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include <boost/asio.hpp>

// *********************************************************************
// ||                             DEFINES                             ||
// *********************************************************************
using namespace boost::asio;

// *********************************************************************
// ||                            FUNCTIONS                            ||
// *********************************************************************

// Given two input matrices, convert them to a string that can be used in a POST request. Kinda gross but it works.
inline std::string convert_txt_matrices_to_http_req(const std::vector<std::vector<std::string>>& matrix1, const std::vector<std::vector<std::string>>& matrix2, bool conv)
{
    std::string http_req = "POST /matrices HTTP/1.1\n";
    http_req += "Host: localhost:45067\n";
    http_req += "Content-Type: application/json\n\n";
    if (conv)
    {
        http_req += "{\"convmatrices\":[";
    }
    else{
        http_req += "{\"matrices\":[";
    }
    http_req += "[";
    for (const auto& row : matrix1)
    {
        http_req += "[";
        for (const auto& cell : row)
        {
            http_req += "\"" + cell + "\",";
        }
        http_req.pop_back();
        http_req += "],";
    }
    http_req.pop_back();
    http_req += "],[";
    for (const auto& row : matrix2)
    {
        http_req += "[";
        for (const auto& cell : row)
        {
            http_req += "\"" + cell + "\",";
        }
        http_req.pop_back();
        http_req += "],";
    }
    http_req.pop_back();
    http_req += "]]}";

    return http_req;
}

inline void handle_options_request(ip::tcp::socket& socket)
{
    // Send CORS headers for preflight request
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: DELETE, POST, GET, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\n";
    response += "Content-Length: 0\r\n\r\n";
    boost::asio::write(socket, boost::asio::buffer(response));
}

inline void send_good_response(ip::tcp::socket& socket, const std::string& response)
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

inline void send_bad_response(ip::tcp::socket& socket, const std::string& response)
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

inline void send_method_not_allowed_response(ip::tcp::socket& socket)
{
    std::string response = "HTTP/1.1 405 Method Not Allowed\r\n";
    response += "Content-Type: text/plain\r\n";
    response += "Access-Control-Allow-Origin: *\r\n"; // Enable CORS for all origins
    response += "Content-Length: 0\r\n\r\n";
    boost::asio::write(socket, boost::asio::buffer(response));
}

#endif
