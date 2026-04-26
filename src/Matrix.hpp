#pragma once

#include "MatrixConstRowProxy.hpp"
#include "MatrixFwd.hpp"
#include "MatrixIterator.hpp"
#include "MatrixRowProxy.hpp"
#include "MatrixStorage.hpp"

#include <cstddef>

template <typename T, T DefaultValue>
class Matrix
{
public:
    using index_type        = int;
    using stored_value_type = T;
    using storage_type      = MatrixStorage<T, DefaultValue>;
    using RowProxy          = MatrixRowProxy<T, DefaultValue>;
    using ConstRowProxy     = MatrixConstRowProxy<T, DefaultValue>;
    using iterator          = MatrixIterator<T, DefaultValue>;

    RowProxy operator[](index_type x)
    {
        return RowProxy{storage_, x};
    }

    ConstRowProxy operator[](index_type x) const
    {
        return ConstRowProxy{storage_, x};
    }

    std::size_t size() const
    {
        return storage_.size();
    }

    iterator begin() const
    {
        return iterator{storage_.begin()};
    }

    iterator end() const
    {
        return iterator{storage_.end()};
    }

private:
    storage_type storage_;
};