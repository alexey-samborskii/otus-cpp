#pragma once

#include "MatrixStorage.hpp"

template <typename T, T DefaultValue>
class MatrixCellProxy
{
public:
    using index_type        = int;
    using stored_value_type = T;
    using storage_type      = MatrixStorage<T, DefaultValue>;

    MatrixCellProxy(storage_type& storage, index_type x, index_type y)
        : storage_(storage)
        , x_(x)
        , y_(y)
    {
    }

    MatrixCellProxy& operator=(const stored_value_type& value)
    {
        storage_.set(x_, y_, value);
        return *this;
    }

    MatrixCellProxy& operator=(const MatrixCellProxy& other)
    {
        return *this = static_cast<stored_value_type>(other);
    }

    operator stored_value_type() const
    {
        return storage_.get(x_, y_);
    }

private:
    storage_type& storage_;
    index_type    x_{};
    index_type    y_{};
};