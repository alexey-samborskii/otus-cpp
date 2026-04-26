#pragma once

#include "MatrixCellProxy.hpp"
#include "MatrixFwd.hpp"

template <typename T, T DefaultValue>
class MatrixRowProxy
{
public:
    using index_type = int;

    MatrixRowProxy(Matrix<T, DefaultValue>& matrix, index_type x)
        : matrix_(matrix)
        , x_(x)
    {
    }

    MatrixCellProxy<T, DefaultValue> operator[](index_type y)
    {
        return MatrixCellProxy<T, DefaultValue>{matrix_, x_, y};
    }

private:
    Matrix<T, DefaultValue>& matrix_;
    index_type               x_{};
};