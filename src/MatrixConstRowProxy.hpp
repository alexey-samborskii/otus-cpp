#pragma once

#include "MatrixStorage.hpp"

#include <cassert>
#include <memory>
#include <utility>

template <typename T, T DefaultValue>
class MatrixConstRowProxy
{
public:
    using index_type        = int;
    using stored_value_type = T;
    using storage_type      = MatrixStorage<T, DefaultValue>;

    MatrixConstRowProxy(std::shared_ptr<const storage_type> storage, index_type x)
        : storage_(std::move(storage))
        , x_(x)
    {
        assert(storage_);
    }

    stored_value_type operator[](index_type y) const
    {
        return storage_->get(x_, y);
    }

private:
    std::shared_ptr<const storage_type> storage_;
    index_type                          x_{};
};