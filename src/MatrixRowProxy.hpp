#pragma once

#include "MatrixCellProxy.hpp"
#include "MatrixStorage.hpp"

template <typename T, T DefaultValue>
class MatrixRowProxy
{
public:
    using index_type   = int;
    using storage_type = MatrixStorage<T, DefaultValue>;
    using CellProxy    = MatrixCellProxy<T, DefaultValue>;

    MatrixRowProxy(storage_type& storage, index_type x)
        : storage_(storage)
        , x_(x)
    {
    }

    CellProxy operator[](index_type y)
    {
        return CellProxy{storage_, x_, y};
    }

private:
    storage_type& storage_;
    index_type    x_{};
};