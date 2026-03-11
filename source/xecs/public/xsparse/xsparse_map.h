#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

template <typename Key, typename T, typename Allocator = std::allocator<T>>
class sparse_map
{
    static_assert(std::is_integral_v<Key>, "sparse_map requires an integral key type.");
    static_assert(std::is_same_v<typename std::allocator_traits<Allocator>::value_type, T>, "Allocator value_type must match sparse_map mapped_type.");

public:
    using key_type                = Key;
    using mapped_type             = T;
    using value_type              = mapped_type;
    using size_type               = std::size_t;
    using allocator_type          = Allocator;
    using key_allocator_type      = typename std::allocator_traits<allocator_type>::template rebind_alloc<key_type>;
    using sparse_allocator_type   = typename std::allocator_traits<allocator_type>::template rebind_alloc<size_type>;
    using dense_key_container     = std::vector<key_type, key_allocator_type>;
    using dense_value_container   = std::vector<mapped_type, allocator_type>;
    using sparse_container        = std::vector<size_type, sparse_allocator_type>;

    static constexpr size_type invalid_index = std::numeric_limits<size_type>::max();

public:
    /// @brief Constructs an empty sparse_map.
    sparse_map() = default;

    /// @brief Constructs an empty sparse_map using a custom allocator.
    /// @param allocator Allocator used for dense value storage and rebound sparse/key storage.
    explicit sparse_map(const allocator_type& allocator)
        : m_sparse(sparse_allocator_type(allocator))
        , m_dense_keys(key_allocator_type(allocator))
        , m_dense_values(allocator)
    {
    }

    /// @brief Constructs an empty sparse_map, reserves dense capacity, and sets allocator.
    /// @param capacity Dense storage capacity to reserve.
    /// @param allocator Allocator used for dense value storage and rebound sparse/key storage.
    explicit sparse_map(size_type capacity, const allocator_type& allocator = allocator_type())
        : m_sparse(sparse_allocator_type(allocator))
        , m_dense_keys(key_allocator_type(allocator))
        , m_dense_values(allocator)
    {
        reserve(capacity);
    }

    /// @brief Reserves capacity in dense storage vectors.
    /// @param capacity Minimum dense capacity after the call.
    /// @return No return value.
    void reserve(size_type capacity)
    {
        m_dense_keys.reserve(capacity);
        m_dense_values.reserve(capacity);
    }

    /// @brief Removes all key/value pairs from the map.
    /// @return No return value.
    void clear()
    {
        for (const key_type key : m_dense_keys)
        {
            const size_type sparse_index = to_index(key);
            m_sparse[sparse_index] = invalid_index;
        }

        m_dense_keys.clear();
        m_dense_values.clear();
    }

    /// @brief Checks whether the map has no elements.
    /// @return true if the map is empty; otherwise false.
    bool empty() const
    {
        return m_dense_values.empty();
    }

    /// @brief Returns the number of elements in the map.
    /// @return Number of stored elements.
    size_type size() const
    {
        return m_dense_values.size();
    }

    /// @brief Checks whether a key exists in the map.
    /// @param key Key to check.
    /// @return true if the key is present; otherwise false.
    bool contains(key_type key) const
    {
        size_type dense_index = invalid_index;
        return try_get_dense_index(key, dense_index);
    }

    /// @brief Returns mutable pointer to mapped value for key, or nullptr if missing.
    /// @param key Key to find.
    /// @return Pointer to mapped value when present; otherwise nullptr.
    mapped_type* find(key_type key)
    {
        size_type dense_index = invalid_index;
        if (!try_get_dense_index(key, dense_index))
        {
            return nullptr;
        }

        return &m_dense_values[dense_index];
    }

    /// @brief Returns read-only pointer to mapped value for key, or nullptr if missing.
    /// @param key Key to find.
    /// @return Pointer to mapped value when present; otherwise nullptr.
    const mapped_type* find(key_type key) const
    {
        size_type dense_index = invalid_index;
        if (!try_get_dense_index(key, dense_index))
        {
            return nullptr;
        }

        return &m_dense_values[dense_index];
    }

    /// @brief Provides mutable mapped access by key, inserting default value when absent.
    /// @param key Key to access.
    /// @return Reference to mapped value.
    mapped_type& operator[](key_type key)
    {
        size_type dense_index = invalid_index;
        if (try_get_dense_index(key, dense_index))
        {
            return m_dense_values[dense_index];
        }

        assert(is_key_valid(key));
        ensure_sparse_size(key);

        m_dense_values.emplace_back();
        try
        {
            m_dense_keys.push_back(key);
        }
        catch (...)
        {
            m_dense_values.pop_back();
            throw;
        }

        dense_index = m_dense_values.size() - 1;
        m_sparse[to_index(key)] = dense_index;

        return m_dense_values[dense_index];
    }

    /// @brief Provides mutable mapped access by key. Key must exist.
    /// @param key Existing key.
    /// @return Reference to mapped value.
    mapped_type& at(key_type key)
    {
        mapped_type* value = find(key);
        assert(value != nullptr);
        return *value;
    }

    /// @brief Provides const mapped access by key. Key must exist.
    /// @param key Existing key.
    /// @return Const reference to mapped value.
    const mapped_type& at(key_type key) const
    {
        const mapped_type* value = find(key);
        assert(value != nullptr);
        return *value;
    }

    /// @brief Inserts key/value pair if key does not already exist.
    /// @param key Key to insert.
    /// @param value Value to insert.
    /// @return true if insertion happened; false if key already existed.
    bool insert(key_type key, const mapped_type& value)
    {
        return emplace(key, value);
    }

    /// @brief Inserts key/value pair if key does not already exist.
    /// @param key Key to insert.
    /// @param value Value to move into storage.
    /// @return true if insertion happened; false if key already existed.
    bool insert(key_type key, mapped_type&& value)
    {
        return emplace(key, std::move(value));
    }

    /// @brief Inserts a key and constructs mapped value in-place when key is absent.
    /// @tparam Args Constructor argument types for mapped value.
    /// @param key Key to insert.
    /// @param args Arguments used to construct mapped value.
    /// @return true if insertion happened; false if key already existed.
    template <typename... Args>
    bool emplace(key_type key, Args&&... args)
    {
        size_type dense_index = invalid_index;
        if (try_get_dense_index(key, dense_index))
        {
            return false;
        }

        assert(is_key_valid(key));
        ensure_sparse_size(key);

        m_dense_values.emplace_back(std::forward<Args>(args)...);
        try
        {
            m_dense_keys.push_back(key);
        }
        catch (...)
        {
            m_dense_values.pop_back();
            throw;
        }

        m_sparse[to_index(key)] = m_dense_values.size() - 1;
        return true;
    }

    /// @brief Inserts or assigns value for key.
    /// @param key Key to insert or assign.
    /// @param value Value to store.
    /// @return true if key was newly inserted; false if existing value was assigned.
    bool insert_or_assign(key_type key, const mapped_type& value)
    {
        if (mapped_type* existing = find(key))
        {
            *existing = value;
            return false;
        }

        (void)emplace(key, value);
        return true;
    }

    /// @brief Inserts or assigns value for key.
    /// @param key Key to insert or assign.
    /// @param value Value to move into storage.
    /// @return true if key was newly inserted; false if existing value was assigned.
    bool insert_or_assign(key_type key, mapped_type&& value)
    {
        if (mapped_type* existing = find(key))
        {
            *existing = std::move(value);
            return false;
        }

        (void)emplace(key, std::move(value));
        return true;
    }

    /// @brief Removes a key/value pair if key exists.
    /// @param key Key to remove.
    /// @return true if the key was removed; false if key was not present.
    bool erase(key_type key)
    {
        size_type dense_index = invalid_index;
        if (!try_get_dense_index(key, dense_index))
        {
            return false;
        }

        const size_type last_index = m_dense_values.size() - 1;

        if (dense_index != last_index)
        {
            const key_type swapped_key = m_dense_keys[last_index];
            m_dense_values[dense_index] = std::move(m_dense_values[last_index]);
            m_dense_keys[dense_index] = swapped_key;
            m_sparse[to_index(swapped_key)] = dense_index;
        }

        m_dense_values.pop_back();
        m_dense_keys.pop_back();
        m_sparse[to_index(key)] = invalid_index;

        return true;
    }

    /// @brief Returns read-only pointer to dense mapped storage.
    /// @return Pointer to first mapped element, or nullptr when empty.
    const mapped_type* data() const
    {
        return m_dense_values.data();
    }

    /// @brief Returns mutable pointer to dense mapped storage.
    /// @return Pointer to first mapped element, or nullptr when empty.
    mapped_type* data()
    {
        return m_dense_values.data();
    }

    /// @brief Returns read-only pointer to dense key storage.
    /// @return Pointer to first key element, or nullptr when empty.
    const key_type* keys_data() const
    {
        return m_dense_keys.data();
    }

    /// @brief Returns mutable pointer to dense key storage.
    /// @return Pointer to first key element, or nullptr when empty.
    key_type* keys_data()
    {
        return m_dense_keys.data();
    }

    /// @brief Returns key at dense index.
    /// @param index Dense index in range [0, size()).
    /// @return Key stored at dense index.
    const key_type& key_at(size_type index) const
    {
        assert(index < m_dense_keys.size());
        return m_dense_keys[index];
    }

    /// @brief Provides const mapped access by dense index.
    /// @param index Dense index in range [0, size()).
    /// @return Const reference to mapped value at index.
    const mapped_type& value_at(size_type index) const
    {
        assert(index < m_dense_values.size());
        return m_dense_values[index];
    }

    /// @brief Provides mutable mapped access by dense index.
    /// @param index Dense index in range [0, size()).
    /// @return Mutable reference to mapped value at index.
    mapped_type& value_at(size_type index)
    {
        assert(index < m_dense_values.size());
        return m_dense_values[index];
    }

    /// @brief Returns iterator to dense mapped storage begin.
    /// @return Mutable begin iterator.
    auto begin() { return m_dense_values.begin(); }

    /// @brief Returns iterator to dense mapped storage end.
    /// @return Mutable end iterator.
    auto end()   { return m_dense_values.end(); }

    /// @brief Returns const iterator to dense mapped storage begin.
    /// @return Const begin iterator.
    auto begin() const { return m_dense_values.begin(); }

    /// @brief Returns const iterator to dense mapped storage end.
    /// @return Const end iterator.
    auto end()   const { return m_dense_values.end(); }

    /// @brief Returns const iterator to dense mapped storage begin.
    /// @return Const cbegin iterator.
    auto cbegin() const { return m_dense_values.cbegin(); }

    /// @brief Returns const iterator to dense mapped storage end.
    /// @return Const cend iterator.
    auto cend()   const { return m_dense_values.cend(); }

private:
    static constexpr bool is_key_signed = std::is_signed_v<key_type>;

    /// @brief Checks whether key can be represented as sparse index.
    static bool is_key_valid(key_type key)
    {
        if constexpr (is_key_signed)
        {
            return key >= 0;
        }
        else
        {
            (void)key;
            return true;
        }
    }

    /// @brief Converts key to sparse index.
    static size_type to_index(key_type key)
    {
        return static_cast<size_type>(key);
    }

    /// @brief Grows sparse storage to represent key.
    void ensure_sparse_size(key_type key)
    {
        const size_type sparse_index = to_index(key);
        if (sparse_index >= m_sparse.size())
        {
            m_sparse.resize(sparse_index + 1, invalid_index);
        }
    }

    /// @brief Resolves dense index for key if present.
    bool try_get_dense_index(key_type key, size_type& dense_index) const
    {
        if (!is_key_valid(key))
        {
            dense_index = invalid_index;
            return false;
        }

        const size_type sparse_index = to_index(key);
        if (sparse_index >= m_sparse.size())
        {
            dense_index = invalid_index;
            return false;
        }

        dense_index = m_sparse[sparse_index];
        return dense_index != invalid_index && dense_index < m_dense_keys.size() && m_dense_keys[dense_index] == key;
    }

private:
    sparse_container      m_sparse;
    dense_key_container   m_dense_keys;
    dense_value_container m_dense_values;
};
