#include "Matrix.hpp"

#include <cassert>
#include <iostream>

//------------------------------------------------------------------------------

template <typename T, T DefaultValue>
void printMatrixFragment(
    const Matrix<T, DefaultValue>& matrix,
    int                            x_from,
    int                            x_to,
    int                            y_from,
    int                            y_to)
{
    std::cout << "matrix "
              << "[" << x_from << ".." << x_to << "]"
              << "[" << y_from << ".." << y_to << "]"
              << ":\n";
    for (int x = x_from; x <= x_to; ++x)
    {
        std::cout << "  ";
        for (int y = y_from; y <= y_to; ++y)
        {
            std::cout << matrix[x][y];
            if (y != y_to)
                std::cout << ' ';
        }
        std::cout << '\n';
    }
}

//------------------------------------------------------------------------------

template <typename T, T DefaultValue>
void printOccupiedCells(const Matrix<T, DefaultValue>& matrix)
{
    std::cout << "\nmatrix.size()==(" << matrix.size() << ")\n\n";
    std::cout << "cells:\n";
    for (const auto& cell : matrix)
    {
        const auto [x, y, value] = cell;
        std::cout << "    [" << x << ',' << y << "]=(" << value << ")\n";
    }
}

//------------------------------------------------------------------------------

void runSelfTest()
{
    Matrix<int, -1> matrix;

    assert(matrix.size() == 0);

    const auto a = matrix[0][0];

    assert(a == -1);
    assert(matrix.size() == 0);

    matrix[100][100] = 314;

    assert(matrix[100][100] == 314);
    assert(matrix.size() == 1);

    ((matrix[100][100] = 314) = 0) = 217;

    assert(matrix[100][100] == 217);
    assert(matrix.size() == 1);

    for (auto c : matrix)
    {
        const auto [x, y, v] = c;
        std::cout << x << y << v << std::endl;
    }

    std::cout << "self-test:\n";
    std::cout << "    ok\n\n";
}

//------------------------------------------------------------------------------

int main()
{
    runSelfTest();

    Matrix<int, 0> matrix;
    for (int i = 0; i < 10; ++i)
    {
        matrix[i][i]     = i;
        matrix[i][9 - i] = 9 - i;
    }
    printMatrixFragment(matrix, 1, 8, 1, 8);
    printOccupiedCells(matrix);

    return 0;
}

//------------------------------------------------------------------------------