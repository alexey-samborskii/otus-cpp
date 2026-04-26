#pragma once

#include "MatrixFwd.hpp"
#include "MatrixConstRowProxy.hpp"
#include "MatrixIterator.hpp"
#include "MatrixRowProxy.hpp"

#include <cstddef>
#include <map>
#include <utility>

template <typename T, T DefaultValue>
class Matrix
{
public:
    using index_type        = int;
    using key_type          = std::pair<index_type, index_type>;
    using stored_value_type = T;
    using RowProxy          = MatrixRowProxy<T, DefaultValue>;
    using ConstRowProxy     = MatrixConstRowProxy<T, DefaultValue>;
    using iterator          = MatrixIterator<T, DefaultValue>;

public:
    RowProxy operator[](index_type x)
    {
        return RowProxy{*this, x};
    }

    ConstRowProxy operator[](index_type x) const
    {
        return ConstRowProxy{*this, x};
    }

    std::size_t size() const
    {
        return data_.size();
    }

    iterator begin() const
    {
        return iterator{data_.cbegin()};
    }

    iterator end() const
    {
        return iterator{data_.cend()};
    }

private:
    stored_value_type get(index_type x, index_type y) const
    {
        const auto it = data_.find({x, y});
        if (it == data_.end())
        {
            return DefaultValue;
        }
        return it->second;
    }

    void set(index_type x, index_type y, const stored_value_type& value)
    {
        const key_type key{x, y};
        if (value == DefaultValue)
        {
            data_.erase(key);
            return;
        }
        data_[key] = value;
    }

    friend class MatrixCellProxy<T, DefaultValue>;
    friend class MatrixConstRowProxy<T, DefaultValue>;

private:
    std::map<key_type, stored_value_type> data_;
};