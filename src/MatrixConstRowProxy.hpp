#pragma once

#include "MatrixStorage.hpp"

template <typename T, T DefaultValue>
class MatrixConstRowProxy
{
public:
    using index_type        = int;
    using stored_value_type = T;
    using storage_type      = MatrixStorage<T, DefaultValue>;

    MatrixConstRowProxy(const storage_type& storage, index_type x)
        : storage_(storage)
        , x_(x)
    {
    }

    stored_value_type operator[](index_type y) const
    {
        return storage_.get(x_, y);
    }

private:
    const storage_type& storage_;
    index_type          x_{};
};