#pragma once

#include "MatrixCellProxy.hpp"
#include "MatrixStorage.hpp"

#include <cassert>
#include <memory>

template <typename T, T DefaultValue>
class MatrixRowProxy
{
public:
    using index_type   = int;
    using storage_type = MatrixStorage<T, DefaultValue>;
    using CellProxy    = MatrixCellProxy<T, DefaultValue>;

    MatrixRowProxy(std::shared_ptr<storage_type> storage, index_type x)
        : storage_(std::move(storage))
        , x_(x)
    {
        assert(storage_);
    }

    CellProxy operator[](index_type y)
    {
        return CellProxy{storage_, x_, y};
    }

private:
    std::shared_ptr<storage_type> storage_;
    index_type                    x_{};
};