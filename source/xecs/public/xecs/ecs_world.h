#pragma once

#include "xecs/ecs_entity.h"
#include "xecs/ecs_system.h"

#include "xsparse/xsparse_map.h"
#include "xsparse/xsparse_set.h"

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace xecs
{
    //--------------------------------------------------------------
    /// @brief Returns the identifier for an entity handle.
    /// @param entity Entity handle to inspect.
    /// @return Entity identifier value.
    inline size_t get_entity_id(const entity& entity);

    //--------------------------------------------------------------
    /// @brief Allocates a unique component type identifier.
    /// @return Newly allocated component type identifier.
    inline size_t allocate_component_type_id();

    //--------------------------------------------------------------
    /// @brief Returns the stable identifier for a component type.
    /// @tparam t_component_type Component type to resolve.
    /// @return Stable component type identifier.
    template <typename t_component_type>
    size_t get_component_id();

    class world
    {
    public:
        //--------------------------------------------------------------
        /// @brief Creates a new entity identifier.
        /// @return Newly allocated entity identifier.
        size_t create_entity();

        //--------------------------------------------------------------
        /// @brief Registers a callable system for a component query.
        /// @tparam t_component_types Component types required by the system.
        /// @tparam t_callable Callable type to store.
        /// @param callable Callable instance to register.
        /// @return Handle to the registered system.
        template <typename... t_component_types, typename t_callable>
        system register_system(t_callable&& callable);

        //--------------------------------------------------------------
        /// @brief Executes a specific registered system.
        /// @param system System handle to execute.
        /// @return No return value.
        void execute_system(const system& system);

        //--------------------------------------------------------------
        /// @brief Executes all registered systems.
        /// @return No return value.
        void execute_all_systems();

        //--------------------------------------------------------------
        /// @brief Iterates all entities that own the requested component set.
        /// @tparam t_component_types Component types required by the query.
        /// @tparam t_function Callable type to invoke for each match.
        /// @param function Callable that receives `(entity, components...)`.
        /// @return No return value.
        template <typename... t_component_types, typename t_function>
        void each(t_function&& function);

        //--------------------------------------------------------------
        /// @brief Adds a component instance to an entity.
        /// @tparam t_component_type Component type to store.
        /// @param entity Entity that receives the component.
        /// @param component Component value to copy into storage.
        /// @return `true` when the component was inserted; otherwise `false`.
        template <typename t_component_type>
        bool add_component(const entity& entity, const t_component_type& component);

        //--------------------------------------------------------------
        /// @brief Removes a component from an entity.
        /// @tparam t_component_type Component type to remove.
        /// @param entity Entity to modify.
        /// @return `true` when the component existed and was removed; otherwise `false`.
        template <typename t_component_type>
        bool remove_component(const entity& entity);

        //--------------------------------------------------------------
        /// @brief Returns a mutable component pointer for an entity.
        /// @tparam t_component_type Component type to fetch.
        /// @param entity Entity to inspect.
        /// @return Pointer to the stored component, or `nullptr` when missing.
        template <typename t_component_type>
        t_component_type* get_component(const entity& entity);

        //--------------------------------------------------------------
        /// @brief Returns a read-only component pointer for an entity.
        /// @tparam t_component_type Component type to fetch.
        /// @param entity Entity to inspect.
        /// @return Pointer to the stored component, or `nullptr` when missing.
        template <typename t_component_type>
        const t_component_type* get_component(const entity& entity) const;

        //--------------------------------------------------------------
        /// @brief Checks whether an entity owns a component type.
        /// @tparam t_component_type Component type to test.
        /// @param entity Entity to inspect.
        /// @return `true` when the component exists; otherwise `false`.
        template <typename t_component_type>
        bool has_component(const entity& entity) const;

    private:
        //--------------------------------------------------------------
        struct ipool
        {
            virtual ~ipool() = default;

            /// @brief Returns the number of entities stored in the pool.
            /// @return Number of stored entities.
            virtual size_t size() const = 0;

            /// @brief Returns the entity identifier at a dense index.
            /// @param index Dense index to inspect.
            /// @return Entity identifier at the requested index.
            virtual size_t entity_at(size_t index) const = 0;
        };

        //--------------------------------------------------------------
        template <typename t_component_type>
        struct pool final : public ipool
        {
            //--------------------------------------------------------------
            /// @brief Returns the number of entities stored in the pool.
            /// @return Number of stored entities.
            size_t size() const override;

            //--------------------------------------------------------------
            /// @brief Returns the entity identifier at a dense index.
            /// @param index Dense index to inspect.
            /// @return Entity identifier at the requested index.
            size_t entity_at(size_t index) const override;

            sparse_set<size_t> entities;
            std::vector<t_component_type> components;
        };

        //--------------------------------------------------------------
        /// @brief Returns an existing pool for a component type or creates one.
        /// @tparam t_component_type Component type to resolve.
        /// @return Reference to the component pool.
        template <typename t_component_type>
        pool<t_component_type>& get_or_create_pool();

        //--------------------------------------------------------------
        /// @brief Returns a read-only component pool when it exists.
        /// @tparam t_component_type Component type to resolve.
        /// @return Pointer to the pool, or `nullptr` when missing.
        template <typename t_component_type>
        const pool<t_component_type>* get_pool() const;

        //--------------------------------------------------------------
        /// @brief Returns a mutable component pool when it exists.
        /// @tparam t_component_type Component type to resolve.
        /// @return Pointer to the pool, or `nullptr` when missing.
        template <typename t_component_type>
        pool<t_component_type>* get_pool();

        //--------------------------------------------------------------
        /// @brief Creates an entity wrapper around an existing identifier.
        /// @param entity_id Existing entity identifier.
        /// @return Entity handle bound to this world.
        entity make_entity(size_t entity_id);

        //--------------------------------------------------------------
        /// @brief Returns the smallest existing pool from a component set.
        /// @tparam t_component_types Component types to evaluate.
        /// @return Pointer to the smallest pool, or `nullptr` when none exist.
        template <typename... t_component_types>
        const ipool* get_smallest_pool() const;

        //--------------------------------------------------------------
        /// @brief Stores a registered system implementation.
        /// @param system_id Identifier used to store the system.
        /// @param system System implementation instance to own.
        /// @return No return value.
        void register_system_impl(size_t system_id, std::unique_ptr<isystem> system);

    private:
        sparse_map<size_t, std::unique_ptr<ipool>> m_component_pools;
        sparse_map<size_t, std::unique_ptr<isystem>> m_systems;
    };

    namespace detail
    {
        //--------------------------------------------------------------
        /// @brief Allocates a unique entity identifier.
        /// @return Newly allocated entity identifier.
        inline size_t allocate_entity_id()
        {
            static std::atomic<size_t> counter{ 0 };
            return counter.fetch_add(1, std::memory_order_relaxed);
        }

        //--------------------------------------------------------------
        /// @brief Allocates a unique system identifier.
        /// @return Newly allocated system identifier.
        inline size_t allocate_system_type_id()
        {
            static std::atomic<size_t> counter{ 0 };
            return counter.fetch_add(1, std::memory_order_relaxed);
        }

        template <typename t_callable, typename... t_component_types>
        class system_model final : public isystem
        {
        public:
            //--------------------------------------------------------------
            /// @brief Stores the callable used by a registered system.
            /// @param callable Callable object to execute for each matching entity.
            template <typename t_callable_arg>
            explicit system_model(t_callable_arg&& callable)
                : m_callable(std::forward<t_callable_arg>(callable))
            {
            }

            //--------------------------------------------------------------
            /// @brief Executes the registered callable for each matching entity.
            /// @param world World instance to process.
            /// @return No return value.
            void execute(world& world) override
            {
                world.each<t_component_types...>(
                    [this](entity current_entity, t_component_types&... components)
                    {
                        invoke(current_entity, components...);
                    });
            }

        private:
            //--------------------------------------------------------------
            /// @brief Invokes the stored callable with either entity-plus-components or components only.
            /// @param current_entity Entity being processed.
            /// @param components Component references for the current entity.
            /// @return No return value.
            void invoke(entity current_entity, t_component_types&... components)
            {
                if constexpr (std::is_invocable_v<t_callable&, entity, t_component_types&...>)
                {
                    std::invoke(m_callable, current_entity, components...);
                }
                else
                {
                    static_assert(std::is_invocable_v<t_callable&, t_component_types&...>,
                        "Systems must be invocable with either (entity, components...) or (components...).");
                    std::invoke(m_callable, components...);
                }
            }

        private:
            t_callable m_callable;
        };
    }

    //==============================================================
    // Free Function Definitions
    //==============================================================

    //--------------------------------------------------------------
    inline size_t get_entity_id(const entity& entity)
    {
        return entity.id();
    }

    //--------------------------------------------------------------
    inline size_t allocate_component_type_id()
    {
        static std::atomic<size_t> counter{ 0 };
        return counter.fetch_add(1, std::memory_order_relaxed);
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    size_t get_component_id()
    {
        static const size_t id = allocate_component_type_id();
        return id;
    }

    //==============================================================
    // world Public Function Definitions
    //==============================================================

    //--------------------------------------------------------------
    inline size_t world::create_entity()
    {
        return detail::allocate_entity_id();
    }

    //--------------------------------------------------------------
    template <typename... t_component_types, typename t_callable>
    inline system world::register_system(t_callable&& callable)
    {
        static_assert(sizeof...(t_component_types) > 0, "Systems require at least one component type.");

        const size_t system_id = detail::allocate_system_type_id();
        using callable_type = std::decay_t<t_callable>;
        auto instance = std::make_unique<detail::system_model<callable_type, t_component_types...>>(std::forward<t_callable>(callable));
        register_system_impl(system_id, std::move(instance));
        return system(this, system_id);
    }

    //--------------------------------------------------------------
    inline void world::execute_system(const system& system)
    {
        auto* instance = m_systems.find(system.id());
        if (instance == nullptr || !(*instance))
        {
            return;
        }

        (*instance)->execute(*this);
    }

    //--------------------------------------------------------------
    inline void world::execute_all_systems()
    {
        for (size_t index = 0; index < m_systems.size(); ++index)
        {
            if (auto& instance = m_systems.value_at(index))
            {
                instance->execute(*this);
            }
        }
    }

    //--------------------------------------------------------------
    template <typename... t_component_types, typename t_function>
    inline void world::each(t_function&& function)
    {
        static_assert(sizeof...(t_component_types) > 0, "Queries require at least one component type.");

        const ipool* candidate_pool = get_smallest_pool<t_component_types...>();
        if (candidate_pool == nullptr)
        {
            return;
        }

        for (size_t index = 0; index < candidate_pool->size(); ++index)
        {
            const entity current_entity = make_entity(candidate_pool->entity_at(index));

            if (!(has_component<t_component_types>(current_entity) && ...))
            {
                continue;
            }

            std::invoke(std::forward<t_function>(function), current_entity, (*get_component<t_component_types>(current_entity))...);
        }
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline bool world::add_component(const entity& entity, const t_component_type& component)
    {
        auto& component_pool = get_or_create_pool<t_component_type>();
        if (!component_pool.entities.insert(get_entity_id(entity)))
        {
            return false;
        }

        component_pool.components.push_back(component);
        return true;
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline bool world::remove_component(const entity& entity)
    {
        auto* component_pool = get_pool<t_component_type>();
        if (component_pool == nullptr)
        {
            return false;
        }

        const size_t entity_id = get_entity_id(entity);
        const size_t index = component_pool->entities.index_of(entity_id);
        if (index == sparse_set<size_t>::invalid_index)
        {
            return false;
        }

        const size_t last_index = component_pool->components.size() - 1;
        if (index != last_index)
        {
            component_pool->components[index] = std::move(component_pool->components[last_index]);
        }

        component_pool->components.pop_back();
        return component_pool->entities.erase(entity_id);
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline t_component_type* world::get_component(const entity& entity)
    {
        auto* component_pool = get_pool<t_component_type>();
        if (component_pool == nullptr)
        {
            return nullptr;
        }

        const size_t index = component_pool->entities.index_of(get_entity_id(entity));
        if (index == sparse_set<size_t>::invalid_index)
        {
            return nullptr;
        }

        return &component_pool->components[index];
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline const t_component_type* world::get_component(const entity& entity) const
    {
        const auto* component_pool = get_pool<t_component_type>();
        if (component_pool == nullptr || !component_pool->entities.contains(get_entity_id(entity)))
        {
            return nullptr;
        }

        const size_t index = component_pool->entities.index_of(get_entity_id(entity));
        if (index == sparse_set<size_t>::invalid_index)
        {
            return nullptr;
        }

        return &component_pool->components[index];
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline bool world::has_component(const entity& entity) const
    {
        const auto* component_pool = get_pool<t_component_type>();
        return component_pool != nullptr && component_pool->entities.contains(get_entity_id(entity));
    }

    //==============================================================
    // world Private Function Definitions
    //==============================================================

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline size_t world::pool<t_component_type>::size() const
    {
        return entities.size();
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline size_t world::pool<t_component_type>::entity_at(size_t index) const
    {
        return entities[index];
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline typename world::template pool<t_component_type>& world::get_or_create_pool()
    {
        const size_t id = get_component_id<t_component_type>();

        auto& slot = m_component_pools[id];
        if (!slot)
        {
            slot = std::make_unique<pool<t_component_type>>();
        }

        return *static_cast<pool<t_component_type>*>(slot.get());
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline const typename world::template pool<t_component_type>* world::get_pool() const
    {
        const size_t id = get_component_id<t_component_type>();
        const auto* slot = m_component_pools.find(id);
        return slot != nullptr ? static_cast<const pool<t_component_type>*>(slot->get()) : nullptr;
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline typename world::template pool<t_component_type>* world::get_pool()
    {
        const size_t id = get_component_id<t_component_type>();
        auto* slot = m_component_pools.find(id);
        return slot != nullptr ? static_cast<pool<t_component_type>*>(slot->get()) : nullptr;
    }

    //--------------------------------------------------------------
    inline entity world::make_entity(size_t entity_id)
    {
        return entity(this, entity_id);
    }

    //--------------------------------------------------------------
    template <typename... t_component_types>
    inline const world::ipool* world::get_smallest_pool() const
    {
        const ipool* smallest = nullptr;

        auto update_smallest =
            [&smallest](const ipool* pool)
            {
                if (pool == nullptr)
                {
                    return;
                }

                if (smallest == nullptr || pool->size() < smallest->size())
                {
                    smallest = pool;
                }
            };

        (update_smallest(get_pool<t_component_types>()), ...);
        return smallest;
    }

    //--------------------------------------------------------------
    inline void world::register_system_impl(size_t system_id, std::unique_ptr<isystem> system)
    {
        m_systems.insert_or_assign(system_id, std::move(system));
    }

    //==============================================================
    // entity Function Definitions
    //==============================================================

    //--------------------------------------------------------------
    inline entity::entity(world* world)
        : m_world(world)
        , m_id(world != nullptr ? world->create_entity() : 0)
    {
    }

    //--------------------------------------------------------------
    inline entity::entity(world* world, size_t id)
        : m_world(world)
        , m_id(id)
    {
    }

    //--------------------------------------------------------------
    inline size_t entity::id() const
    {
        return m_id;
    }

    //--------------------------------------------------------------
    inline bool entity::valid() const
    {
        return m_world != nullptr;
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline bool entity::add_component(const t_component_type& component)
    {
        return m_world != nullptr && m_world->add_component<t_component_type>(*this, component);
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline bool entity::remove_component()
    {
        return m_world != nullptr && m_world->remove_component<t_component_type>(*this);
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline t_component_type* entity::get_component()
    {
        return m_world != nullptr ? m_world->get_component<t_component_type>(*this) : nullptr;
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline const t_component_type* entity::get_component() const
    {
        return m_world != nullptr ? m_world->get_component<t_component_type>(*this) : nullptr;
    }

    //--------------------------------------------------------------
    template <typename t_component_type>
    inline bool entity::has_component() const
    {
        return m_world != nullptr && m_world->has_component<t_component_type>(*this);
    }

    //==============================================================
    // system Function Definitions
    //==============================================================

    //--------------------------------------------------------------
    inline system::system(world* world, size_t id)
        : m_world(world)
        , m_id(id)
    {
    }

    //--------------------------------------------------------------
    inline size_t system::id() const
    {
        return m_id;
    }

    //--------------------------------------------------------------
    inline bool system::valid() const
    {
        return m_world != nullptr;
    }

    //--------------------------------------------------------------
    inline void system::execute() const
    {
        if (m_world != nullptr)
        {
            m_world->execute_system(*this);
        }
    }
}
