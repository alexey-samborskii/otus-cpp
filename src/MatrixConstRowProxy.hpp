#pragma once

#include "MatrixFwd.hpp"

template <typename T, T DefaultValue>
class MatrixConstRowProxy
{
public:
    using index_type        = int;
    using stored_value_type = T;

    MatrixConstRowProxy(const Matrix<T, DefaultValue>& matrix, index_type x)
        : matrix_(matrix)
        , x_(x)
    {
    }

    stored_value_type operator[](index_type y) const
    {
        return matrix_.get(x_, y);
    }

private:
    const Matrix<T, DefaultValue>& matrix_;
    index_type                     x_{};
};