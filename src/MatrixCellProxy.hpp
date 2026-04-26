#pragma once

#include "MatrixFwd.hpp"

template <typename T, T DefaultValue>
class MatrixCellProxy
{
public:
    using index_type        = int;
    using stored_value_type = T;

    MatrixCellProxy(Matrix<T, DefaultValue>& matrix, index_type x, index_type y)
        : matrix_(matrix)
        , x_(x)
        , y_(y)
    {
    }

    MatrixCellProxy& operator=(const stored_value_type& value)
    {
        matrix_.set(x_, y_, value);
        return *this;
    }

    MatrixCellProxy& operator=(const MatrixCellProxy& other)
    {
        return *this = static_cast<stored_value_type>(other);
    }

    operator stored_value_type() const
    {
        return matrix_.get(x_, y_);
    }

private:
    Matrix<T, DefaultValue>& matrix_;
    index_type               x_{};
    index_type               y_{};
};