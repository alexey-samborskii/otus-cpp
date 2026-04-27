#pragma once

#include "MatrixConstRowProxy.hpp"
#include "MatrixFwd.hpp"
#include "MatrixIterator.hpp"
#include "MatrixRowProxy.hpp"
#include "MatrixStorage.hpp"

#include <cstddef>
#include <memory>

template <typename T, T DefaultValue>
class Matrix
{
public:
    using index_type        = int;
    using stored_value_type = T;
    using storage_type      = MatrixStorage<T, DefaultValue>;
    using RowProxy          = MatrixRowProxy<T, DefaultValue>;
    using ConstRowProxy     = MatrixConstRowProxy<T, DefaultValue>;
    using const_iterator    = typename storage_type::const_iterator;

public:
    Matrix()
        : storage_(std::make_shared<storage_type>())
    {
    }

    Matrix(const Matrix& other)
        : storage_(std::make_shared<storage_type>(*other.storage_))
    {
    }

    Matrix& operator=(const Matrix& other)
    {
        if (this == &other)
        {
            return *this;
        }

        storage_ = std::make_shared<storage_type>(*other.storage_);
        return *this;
    }

    Matrix(Matrix&&) noexcept            = default;
    Matrix& operator=(Matrix&&) noexcept = default;

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
        return storage_->size();
    }

    const_iterator begin() const
    {
        return storage_->begin();
    }

    const_iterator end() const
    {
        return storage_->end();
    }

    const_iterator cbegin() const
    {
        return storage_->cbegin();
    }

    const_iterator cend() const
    {
        return storage_->cend();
    }

private:
    std::shared_ptr<storage_type> storage_;
};