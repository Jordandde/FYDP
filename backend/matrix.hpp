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
class Matrix {
    private:
        std::vector<std::vector<float>> matrix;
        int num_rows;
        int num_cols;

    public:
        // Define the iterator for the matrix
        Matrix() {
            num_rows = 0;
            num_cols = 0;
            matrix = std::vector<std::vector<float>>();
        }

        // Float constructor for matrix
        Matrix(std::vector<std::vector<float>> input_matrix) {
            matrix = input_matrix;
            num_rows = input_matrix.size();
            num_cols = input_matrix[0].size();
        }

        // String constructor for matrix
        Matrix(std::vector<std::vector<std::string>> input_matrix) {
            for (const auto& row : input_matrix) {
                std::vector<float> float_row;
                for (const auto& cell : row) {
                    float_row.push_back(std::stof(cell));
                }
                matrix.push_back(float_row);
            }
            num_rows = input_matrix.size();
            num_cols = input_matrix[0].size();
        }

        // Dimensions constructor for matrix
        Matrix(int rows, int cols) {
            num_rows = rows;
            num_cols = cols;
            // Create matrix of zeros
            for (int i = 0; i < rows; i++) {
                std::vector<float> row;
                for (int j = 0; j < cols; j++) {
                    row.push_back(0.0F);
                }
                matrix.push_back(row);
            }
        }

        // Get string representation of matrix
        std::string to_string(void) {
            std::string matrix_str = "";
            for (const auto& row : matrix) {
                for (const auto& cell : row) {
                    matrix_str += std::to_string(cell) + " ";
                }
                matrix_str += "\n";
            }
            return matrix_str;
        }

        // Get number of rows
        int get_num_rows(void) {
            return num_rows;
        }

        // Get number of columns
        int get_num_cols(void) {
            return num_cols;
        }

        // Implement [] operator
        std::vector<float>& operator[](int index) {
            return matrix[index];
        }

        // Make matrix iterable
        std::vector<std::vector<float>>::iterator begin() {
            return matrix.begin();
        }

        std::vector<std::vector<float>>::iterator end() {
            return matrix.end();
        }
};

// Class for storing the matrices
class Matrices {
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
