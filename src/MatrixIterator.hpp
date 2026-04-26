#pragma once

#include "MatrixFwd.hpp"

#include <iterator>
#include <map>
#include <tuple>
#include <utility>

template <typename T, T DefaultValue>
class MatrixIterator
{
public:
    using index_type        = int;
    using stored_value_type = T;
    using key_type          = std::pair<index_type, index_type>;
    using inner_iterator    = typename std::map<key_type, stored_value_type>::const_iterator;

    using iterator_category = std::forward_iterator_tag;
    using value_type        = std::tuple<index_type, index_type, stored_value_type>;
    using difference_type   = std::ptrdiff_t;
    using pointer           = void;
    using reference         = value_type;

    explicit MatrixIterator(inner_iterator it)
        : it_(it)
    {
    }

    value_type operator*() const
    {
        return {
            it_->first.first,
            it_->first.second,
            it_->second};
    }

    MatrixIterator& operator++()
    {
        ++it_;
        return *this;
    }

    MatrixIterator operator++(int)
    {
        MatrixIterator tmp{*this};
        ++(*this);
        return tmp;
    }

    bool operator==(const MatrixIterator& other) const
    {
        return it_ == other.it_;
    }

    bool operator!=(const MatrixIterator& other) const
    {
        return !(*this == other);
    }

private:
    inner_iterator it_;
};