#pragma once

#include <cstddef>
#include <map>
#include <utility>

template <typename T, T DefaultValue>
class MatrixStorage
{
public:
    using index_type        = int;
    using stored_value_type = T;
    using key_type          = std::pair<index_type, index_type>;
    using container_type    = std::map<key_type, stored_value_type>;
    using const_iterator    = typename container_type::const_iterator;

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

    std::size_t size() const
    {
        return data_.size();
    }

    const_iterator begin() const
    {
        return cbegin();
    }

    const_iterator end() const
    {
        return cend();
    }

    const_iterator cbegin() const
    {
        return data_.cbegin();
    }

    const_iterator cend() const
    {
        return data_.cend();
    }

private:
    container_type data_;
};