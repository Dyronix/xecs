#pragma once

#include "xsparse/xsparse_set.h"

#include <unordered_map>
#include <atomic>
#include <memory>

namespace xecs
{
    class entity;

    //--------------------------------------------------------------
    size_t get_entity_id(const entity& entity);

    //--------------------------------------------------------------
    inline size_t allocate_component_type_id()
    {
        static std::atomic<size_t> counter{0};
        return counter.fetch_add(1, std::memory_order_relaxed);
    }

    //--------------------------------------------------------------
    template <typename T>
    size_t get_component_id()
    {
        static const size_t id = allocate_component_type_id();
        return id;
    }

    class world
    {
    public:
        //--------------------------------------------------------------
        size_t create_entity();

        //--------------------------------------------------------------
        template <typename TComponentType>
        bool add_component(const entity &entity, const TComponentType &component)
        {
            auto& pool = get_or_create_pool<TComponentType>();
            if (!pool.entities.insert(get_entity_id(entity)))
            {
                return false;
            }

            pool.components.push_back(component);

            return true;
        }

        //--------------------------------------------------------------
        template <typename TComponentType>
        bool remove_component(const entity &entity)
        {
            auto& pool = get_or_create_pool<TComponentType>();

            return pool.entities.erase(get_entity_id(entity));

            // Note: Component data is not removed from the pool to avoid fragmentation.
        }

        //--------------------------------------------------------------
        template <typename TComponentType>
        TComponentType* get_component(const entity &entity)
        {
            auto& pool = get_or_create_pool<TComponentType>();

            if (!pool.entities.contains(get_entity_id(entity)))
            {
                return nullptr;
            }

            const size_t index = pool.entities.data()[get_entity_id(entity)];
            return &pool.components[index];
        }

        //--------------------------------------------------------------
        template <typename TComponentType>
        const TComponentType* get_component(const entity &entity) const
        {
            const auto& pool = get_or_create_pool<TComponentType>();

            if (!pool.entities.contains(get_entity_id(entity)))
            {
                return nullptr;
            }

            const size_t index = pool.entities.data()[get_entity_id(entity)];
            return &pool.components[index];
        }

        //--------------------------------------------------------------
        template <typename TComponentType>
        bool has_component(const entity &entity) const
        {
            const auto& pool = get_or_create_pool<TComponentType>();

            return pool.entities.contains(get_entity_id(entity));
        }

    private:
        //--------------------------------------------------------------
        struct ipool
        {
            virtual ~ipool() = default;
        };

        //--------------------------------------------------------------
        template <typename T>
        struct pool final : public ipool
        {
            sparse_set<size_t> entities;
            std::vector<T> components;
        };

        //--------------------------------------------------------------
        template <typename T>
        pool<T>& get_or_create_pool()
        {
            const size_t id = get_component_id<T>();

            auto &slot = m_component_pools[id];
            if (!slot)
            {
                slot = std::make_unique<pool<T>>();
            }

            return *static_cast<pool<T> *>(slot.get());
        }

    private:
        std::unordered_map<size_t, std::unique_ptr<ipool>> m_component_pools;
    };
}
