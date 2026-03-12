#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <cassert>
#include <memory>
#include <type_traits>

template <typename T, typename Allocator = std::allocator<T>>
class sparse_set
{
    static_assert(std::is_integral_v<T>, "sparse_set requires an integral id type.");
    static_assert(std::is_same_v<typename std::allocator_traits<Allocator>::value_type, T>, "Allocator value_type must match sparse_set value_type.");

public:
    using value_type            = T;
    using size_type             = std::size_t;
    using allocator_type        = Allocator;
    using sparse_allocator_type = typename std::allocator_traits<allocator_type>::template rebind_alloc<size_type>;
    using dense_container_type  = std::vector<value_type, allocator_type>;
    using sparse_container_type = std::vector<size_type, sparse_allocator_type>;

    static constexpr size_type invalid_index = std::numeric_limits<size_type>::max();

public:
    /// @brief Constructs an empty sparse_set.
    sparse_set() = default;

    /// @brief Constructs an empty sparse_set using a custom allocator.
    /// @param allocator Allocator used for dense storage and rebound sparse storage.
    explicit sparse_set(const allocator_type& allocator)
        : m_sparse(sparse_allocator_type(allocator))
        , m_dense(allocator)
    {
    }

    /// @brief Constructs an empty sparse_set, reserves dense capacity, and sets allocator.
    /// @param capacity Dense storage capacity to reserve.
    /// @param allocator Allocator used for dense storage and rebound sparse storage.
    explicit sparse_set(size_type capacity, const allocator_type& allocator = allocator_type())
        : m_sparse(sparse_allocator_type(allocator))
        , m_dense(allocator)
    {
        reserve(capacity);
    }

    /// @brief Reserves capacity in the dense storage vector.
    /// @param capacity Minimum dense capacity after the call.
    /// @return No return value.
    void reserve(size_type capacity)
    {
        m_dense.reserve(capacity);
    }

    /// @brief Removes all values from the set.
    /// @return No return value.
    void clear()
    {
        for (const value_type value : m_dense)
        {
            m_sparse[static_cast<size_type>(value)] = invalid_index;
        }

        m_dense.clear();
    }

    /// @brief Checks whether the set has no elements.
    /// @return true if the set is empty; otherwise false.
    bool empty() const
    {
        return m_dense.empty();
    }

    /// @brief Returns the number of elements in the set.
    /// @return Number of stored elements.
    size_type size() const
    {
        return m_dense.size();
    }

    /// @brief Checks whether a value exists in the set.
    /// @param value Value to check.
    /// @return true if the value is present; otherwise false.
    bool contains(value_type value) const
    {
        const size_type index = static_cast<size_type>(value);

        if (index >= m_sparse.size())
        {
            return false;
        }

        const size_type dense_index = m_sparse[index];
        return dense_index != invalid_index && dense_index < m_dense.size() && m_dense[dense_index] == value;
    }

    /// @brief Inserts a value if it does not already exist.
    /// @param value Value to insert.
    /// @return true if insertion happened; false if the value already existed.
    bool insert(value_type value)
    {
        if (contains(value))
        {
            return false;
        }

        ensure_sparse_size(value);

        const size_type dense_index = m_dense.size();
        m_dense.push_back(value);
        m_sparse[static_cast<size_type>(value)] = dense_index;

        return true;
    }

    /// @brief Removes a value if it exists.
    /// @param value Value to remove.
    /// @return true if the value was removed; false if it was not present.
    bool erase(value_type value)
    {
        if (!contains(value))
        {
            return false;
        }

        const size_type sparse_index = static_cast<size_type>(value);
        const size_type dense_index  = m_sparse[sparse_index];
        const size_type last_index   = m_dense.size() - 1;
        const value_type last_value  = m_dense[last_index];

        if (dense_index != last_index)
        {
            m_dense[dense_index] = last_value;
            m_sparse[static_cast<size_type>(last_value)] = dense_index;
        }

        m_dense.pop_back();
        m_sparse[sparse_index] = invalid_index;

        return true;
    }

    /// @brief Returns dense index for a value, or invalid_index when missing.
    /// @param value Value to resolve.
    /// @return Dense index when present; otherwise invalid_index.
    size_type index_of(value_type value) const
    {
        const size_type index = static_cast<size_type>(value);

        if (index >= m_sparse.size())
        {
            return invalid_index;
        }

        const size_type dense_index = m_sparse[index];
        if (dense_index == invalid_index || dense_index >= m_dense.size() || m_dense[dense_index] != value)
        {
            return invalid_index;
        }

        return dense_index;
    }

    /// @brief Returns read-only pointer to dense storage.
    /// @return Pointer to first dense element, or nullptr when empty.
    const value_type* data() const
    {
        return m_dense.data();
    }

    /// @brief Returns mutable pointer to dense storage.
    /// @return Pointer to first dense element, or nullptr when empty.
    value_type* data()
    {
        return m_dense.data();
    }

    /// @brief Provides const element access by dense index.
    /// @param index Dense index in range [0, size()).
    /// @return Const reference to the element at the index.
    const value_type& operator[](size_type index) const
    {
        assert(index < m_dense.size());
        return m_dense[index];
    }

    /// @brief Provides mutable element access by dense index.
    /// @param index Dense index in range [0, size()).
    /// @return Mutable reference to the element at the index.
    value_type& operator[](size_type index)
    {
        assert(index < m_dense.size());
        return m_dense[index];
    }

    /// @brief Returns iterator to the beginning of dense storage.
    /// @return Mutable begin iterator.
    auto begin() { return m_dense.begin(); }

    /// @brief Returns iterator one past the end of dense storage.
    /// @return Mutable end iterator.
    auto end()   { return m_dense.end(); }

    /// @brief Returns const iterator to the beginning of dense storage.
    /// @return Const begin iterator.
    auto begin() const { return m_dense.begin(); }

    /// @brief Returns const iterator one past the end of dense storage.
    /// @return Const end iterator.
    auto end()   const { return m_dense.end(); }

    /// @brief Returns const iterator to the beginning of dense storage.
    /// @return Const cbegin iterator.
    auto cbegin() const { return m_dense.cbegin(); }

    /// @brief Returns const iterator one past the end of dense storage.
    /// @return Const cend iterator.
    auto cend()   const { return m_dense.cend(); }

private:
    /// @brief Grows sparse storage to be able to index the provided value.
    /// @param value Value whose index must be representable.
    /// @return No return value.
    void ensure_sparse_size(value_type value)
    {
        const size_type index = static_cast<size_type>(value);

        if (index >= m_sparse.size())
        {
            m_sparse.resize(index + 1, invalid_index);
        }
    }

private:
    sparse_container_type m_sparse;
    dense_container_type m_dense;
};
