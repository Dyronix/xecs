#pragma once

#include <cstddef>

namespace xecs
{
    class world;

    class entity
    {
    public:
        /// @brief Returns the stable entity identifier.
        /// @return Entity identifier value.
        size_t id() const;

        /// @brief Checks whether the entity references a world.
        /// @return `true` when the entity belongs to a world; otherwise `false`.
        bool valid() const;

        //--------------------------------------------------------------
        /// @brief Adds a component instance to the entity.
        /// @tparam t_component_type Component type to store.
        /// @param component Component value to copy into the world storage.
        /// @return `true` when the component was inserted; otherwise `false`.
        template <typename t_component_type>
        bool add_component(const t_component_type& component);

        //--------------------------------------------------------------
        /// @brief Removes a component from the entity.
        /// @tparam t_component_type Component type to remove.
        /// @return `true` when the component existed and was removed; otherwise `false`.
        template <typename t_component_type>
        bool remove_component();

        //--------------------------------------------------------------
        /// @brief Returns a mutable component pointer for the entity.
        /// @tparam t_component_type Component type to fetch.
        /// @return Pointer to the stored component, or `nullptr` when missing.
        template <typename t_component_type>
        t_component_type* get_component();

        //--------------------------------------------------------------
        /// @brief Returns a read-only component pointer for the entity.
        /// @tparam t_component_type Component type to fetch.
        /// @return Pointer to the stored component, or `nullptr` when missing.
        template <typename t_component_type>
        const t_component_type* get_component() const;

        //--------------------------------------------------------------
        /// @brief Checks whether the entity owns a component type.
        /// @tparam t_component_type Component type to test.
        /// @return `true` when the component exists; otherwise `false`.
        template <typename t_component_type>
        bool has_component() const;

    private:
        friend class world;

        /// @brief Creates a new entity in the provided world.
        /// @param world World that will own the new entity.
        explicit entity(world* world);

        /// @brief Wraps an existing entity identifier.
        /// @param world Owning world.
        /// @param id Existing entity identifier.
        entity(world* world, size_t id);

    private:
        world* m_world{ nullptr };
        size_t m_id{ 0 };
    };
} // namespace xecs
