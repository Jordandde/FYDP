// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include "main.h"
#include <boost/program_options.hpp>

// *********************************************************************
// ||                             DEFINES                             ||
// *********************************************************************
namespace po = boost::program_options;

// *********************************************************************
// ||                           GLOBAL VARS                           ||
// *********************************************************************
bool FPGA_IN_LOOP = true;
bool FRONTEND_IN_LOOP = true;
bool PRINT_REQUEST = false;
bool CALIBRATION_MODE = false;
bool CONV_MODE = false;

float calibration_factor_even = 1.0;
float calibration_factor_odd = 1.0;

std::vector<std::vector<std::string>> fake_input1;
std::vector<std::vector<std::string>> fake_input2;

// *********************************************************************
// ||                           FUNCTIONS                             ||
// *********************************************************************
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
        // Find the position of the first '{' character after the headers
        size_t json_start_pos = request.find("{");
        if (json_start_pos == std::string::npos)
        {
            throw std::invalid_argument("JSON data not found in request.");
        }

        // Extract the JSON payload from the request and parse it into the Matrices struct
        std::string in_json_payload = request.substr(json_start_pos);
        auto in_matrices_payload = nlohmann::json::parse(in_json_payload)["matrices"];
        auto calibration_matrix = nlohmann::json::parse(in_json_payload)["calibrationMatrix"];
        auto conv_matrices = nlohmann::json::parse(in_json_payload)["convmatrices"];

        std::cout << "Matrices received successfully!" << std::endl;
        if (conv_matrices.size() == 2)
        {
            std::cout << "Convolution Mode" << std::endl;
            Matrix kernel(static_cast<std::vector<std::vector<std::string>>>(conv_matrices[0][0]));
            Matrix image(static_cast<std::vector<std::vector<std::string>>>(conv_matrices[1]));
            handle_conv_request(frontend_socket, fpga_socket, kernel, image);
        }
        else if (calibration_matrix.size() == 2)
        {
            std::cout << "Calibration Mode" << std::endl;
            CALIBRATION_MODE = true;
            Matrices matrices(calibration_matrix[0], calibration_matrix[1]);
            handle_matmul_request(frontend_socket, fpga_socket, matrices);
        }
        else if (in_matrices_payload.size() == 2)
        {
            std::cout << "Matrix Multiplication Mode" << std::endl;
            CALIBRATION_MODE = false;
            Matrices matrices(in_matrices_payload[0], in_matrices_payload[1]);
            handle_matmul_request(frontend_socket, fpga_socket, matrices);
        }
        else
        {
            std::cout << "Expected 2 matrices, received " << in_matrices_payload.size() << " matrices." << std::endl;
            return;
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
                        if (closing_pos != std::string::npos)
                        {
                            request.resize(closing_pos + 1);
                        }
                        // Process the request
                        handle_request(request, frontend_socket, fpga_socket);
                    }
                }
                else
                {
                    // Process a dummy request
                    handle_request(convert_txt_matrices_to_http_req(fake_input1, fake_input2, CONV_MODE), frontend_socket, fpga_socket);
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
        ("conv_mode", "whether or not to do convolution on the fake data")
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

    if (vm.count("conv_mode"))
    {
        std::cout << "Convolution mode " << std::endl;
        CONV_MODE = true;
    }
    else
    {
        std::cout << "Matmul mode " << std::endl;
        CONV_MODE = false;
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
