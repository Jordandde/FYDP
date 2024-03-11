#ifndef MATRIX_HPP
#define MATRIX_HPP

// *********************************************************************
// ||                            INCLUDES                             ||
// *********************************************************************
#include <string>
#include <vector>

// *********************************************************************
// ||                             CLASSES                             ||
// *********************************************************************
// Matrix class
class Matrix
{
  private:
    std::vector<std::vector<float>> matrix;
    int num_rows;
    int num_cols;

  public:
    // Default constructor for matrix
    Matrix()
    {
        num_rows = 0;
        num_cols = 0;
        matrix = std::vector<std::vector<float>>();
    }

    // Float constructor for matrix
    Matrix(std::vector<std::vector<float>> input_matrix)
    {
        matrix = input_matrix;
        num_rows = input_matrix.size();
        num_cols = input_matrix[0].size();
    }

    // Float constructor for matrix with transpose option
    Matrix(std::vector<std::vector<float>> input_matrix, bool transpose)
    {
        // Copy in input_matrix transposed
        for (int i = 0; i < input_matrix[0].size(); i++)
        {
            std::vector<float> row;
            for (int j = 0; j < input_matrix.size(); j++)
            {
                row.push_back(input_matrix[j][i]);
            }
            matrix.push_back(row);
        }
        num_rows = input_matrix[0].size();
        num_cols = input_matrix.size();
    }

    // String constructor for matrix
    Matrix(std::vector<std::vector<std::string>> input_matrix)
    {
        for (const auto& row : input_matrix)
        {
            std::vector<float> float_row;
            for (const auto& cell : row)
            {
                float_row.push_back(std::stof(cell));
            }
            matrix.push_back(float_row);
        }
        num_rows = input_matrix.size();
        num_cols = input_matrix[0].size();
    }

    // Dimensions constructor for matrix
    Matrix(int rows, int cols)
    {
        num_rows = rows;
        num_cols = cols;
        // Create matrix of zeros
        for (int i = 0; i < rows; i++)
        {
            std::vector<float> row;
            for (int j = 0; j < cols; j++)
            {
                row.push_back(0.0F);
            }
            matrix.push_back(row);
        }
    }

    // Get string representation of matrix
    std::string to_string(void)
    {
        std::string matrix_str = "";
        for (const auto& row : matrix)
        {
            for (const auto& cell : row)
            {
                matrix_str += std::to_string(cell) + " ";
            }
            matrix_str += "\n";
        }
        return matrix_str;
    }

    // Get string representation of matrix with precision
    std::string to_string_with_precision(const int n = 2)
    {
        std::ostringstream out;
        out.precision(n);
        for (const auto& row : matrix)
        {
            for (const auto& cell : row)
            {
                out << std::fixed << cell;
                out << " ";
            }
            out << "\n";
        }

        return std::move(out).str();
    }

    // Get number of rows
    int get_num_rows(void) { return num_rows; }

    // Get number of columns
    int get_num_cols(void) { return num_cols; }

    // Pad edges of matrix for convolution
    void pad(void) // TODO: Made padding size variable
    {
        // Pad left and right
        for (int i = 0; i < num_rows; i++)
        {
            matrix[i].insert(matrix[i].begin(), matrix[i][0]);
            matrix[i].push_back(matrix[i][num_cols - 1]);
        }
        num_cols = matrix[0].size();

        // Pad top and bottom
        matrix.push_back(matrix[num_rows - 1]);
        matrix.insert(matrix.begin(), matrix[0]);
        num_rows = matrix.size();
    }

    // Implement [] operator
    std::vector<float>& operator[](int index) { return matrix[index]; }

    // Implement * operator to perform matrix multiplication
    Matrix operator*(Matrix& other)
    {
        // Check if the matrices can be multiplied
        if (num_cols != other.get_num_rows())
        {
            throw std::invalid_argument("Matrices cannot be multiplied");
        }

        // Initialize result matrix
        Matrix result(num_rows, other.get_num_cols());

        // Perform matrix multiplication
        for (int i = 0; i < num_rows; i++)
        {
            for (int j = 0; j < other.get_num_cols(); j++)
            {
                for (int k = 0; k < num_cols; k++)
                {
                    result[i][j] += matrix[i][k] * other[k][j];
                }
            }
        }

        return result;
    }

    // Implement - operator to perform matrix subtraction
    Matrix& operator-=(Matrix&& other)
    {
        // Check if the matrices have the same dimensions
        if (num_rows != other.get_num_rows() || num_cols != other.get_num_cols())
        {
            throw std::invalid_argument("Matrices do not have the same dimensions");
        }

        // Perform matrix subtraction
        for (int i = 0; i < num_rows; i++)
        {
            for (int j = 0; j < num_cols; j++)
            {
                matrix[i][j] = matrix[i][j] - other[i][j];
            }
        }

        return *this;
    }

    // Make matrix iterable
    std::vector<std::vector<float>>::iterator begin() { return matrix.begin(); }

    std::vector<std::vector<float>>::iterator end() { return matrix.end(); }
};

// Class for storing the matrices
class Matrices
{
  public:
    Matrix input_matrix_1;
    Matrix input_matrix_2;
    Matrix result_matrix;

    Matrices(std::vector<std::vector<std::string>> input1, std::vector<std::vector<std::string>> input2)
    {
        // Convert input1 and input2 to float and store them in input_matrix_1 and input_matrix_2
        input_matrix_1 = Matrix(input1);
        input_matrix_2 = Matrix(input2);

        // Initialize result_matrix with the correct dimensions

        int num_rows_output = input_matrix_1.get_num_rows();
        int num_cols_output = input_matrix_2.get_num_cols();

        result_matrix = Matrix(num_rows_output, num_cols_output);
    }
};

#endif
