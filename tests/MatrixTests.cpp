#include "Matrix.hpp"

#include <gtest/gtest.h>

//------------------------------------------------------------------------------

TEST(MatrixTest, DefaultValueDoesNotCreateCell)
{
    Matrix<int, -1> matrix;

    EXPECT_EQ(matrix.size(), 0U);

    const auto value = matrix[0][0];

    EXPECT_EQ(value, -1);
    EXPECT_EQ(matrix.size(), 0U);
}

//------------------------------------------------------------------------------

TEST(MatrixTest, SetAndReadCell)
{
    Matrix<int, -1> matrix;

    matrix[100][100] = 314;

    EXPECT_EQ(matrix[100][100], 314);
    EXPECT_EQ(matrix.size(), 1U);
}

//------------------------------------------------------------------------------

TEST(MatrixTest, ChainedAssignment)
{
    Matrix<int, -1> matrix;

    ((matrix[100][100] = 314) = 0) = 217;

    EXPECT_EQ(matrix[100][100], 217);
    EXPECT_EQ(matrix.size(), 1U);
}

//------------------------------------------------------------------------------

TEST(MatrixTest, AssignDefaultValueRemovesCell)
{
    Matrix<int, -1> matrix;

    matrix[100][100] = 314;

    ASSERT_EQ(matrix.size(), 1U);

    matrix[100][100] = -1;

    EXPECT_EQ(matrix[100][100], -1);
    EXPECT_EQ(matrix.size(), 0U);
}

//------------------------------------------------------------------------------

TEST(MatrixTest, IteratesOverOccupiedCells)
{
    Matrix<int, -1> matrix;

    matrix[100][100] = 217;

    std::size_t count = 0;

    for (const auto& [key, value] : matrix)
    {
        const auto& [x, y] = key;

        EXPECT_EQ(x, 100);
        EXPECT_EQ(y, 100);
        EXPECT_EQ(value, 217);

        ++count;
    }

    EXPECT_EQ(count, 1U);
}

//------------------------------------------------------------------------------

TEST(MatrixTest, ConstMatrixRead)
{
    Matrix<int, -1> matrix;

    matrix[10][20] = 42;

    const auto& const_matrix = matrix;

    EXPECT_EQ(const_matrix[10][20], 42);
    EXPECT_EQ(const_matrix[1][2], -1);
}

//------------------------------------------------------------------------------

TEST(MatrixTest, CopyIsDeep)
{
    Matrix<int, -1> matrix;

    matrix[1][2] = 10;

    Matrix<int, -1> copy = matrix;

    copy[1][2] = 20;

    EXPECT_EQ(matrix[1][2], 10);
    EXPECT_EQ(copy[1][2], 20);
}

//------------------------------------------------------------------------------

TEST(MatrixTest, AssignCellFromAnotherCell)
{
    Matrix<int, -1> matrix;

    matrix[1][2] = 10;
    matrix[3][4] = matrix[1][2];

    EXPECT_EQ(matrix[1][2], 10);
    EXPECT_EQ(matrix[3][4], 10);
    EXPECT_EQ(matrix.size(), 2U);
}

//------------------------------------------------------------------------------

TEST(MatrixTest, DefaultValueAssignmentDoesNotCreateNewCell)
{
    Matrix<int, -1> matrix;

    matrix[5][5] = -1;

    EXPECT_EQ(matrix[5][5], -1);
    EXPECT_EQ(matrix.size(), 0U);
}

//------------------------------------------------------------------------------