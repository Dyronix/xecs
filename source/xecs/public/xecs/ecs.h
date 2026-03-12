#pragma once

#include "xecs/ecs_world.h"

namespace xecs
{
    //--------------------------------------------------------------
    /// @brief Creates a new entity in a world through the C-style helper API.
    /// @param world World that will allocate the entity.
    /// @return Newly created entity identifier, or `0` when `world` is `nullptr`.
    size_t ecs_world_create_entity(world* world);

    //--------------------------------------------------------------
    /// @brief Returns the identifier stored in an entity handle.
    /// @param entity Entity handle to inspect.
    /// @return Entity identifier, or `0` when `entity` is `nullptr`.
    size_t ecs_entity_id(const entity* entity);

    //--------------------------------------------------------------
    /// @brief Returns the identifier for an entity through the helper wrapper.
    /// @param entity Entity handle to inspect.
    /// @return Entity identifier, or `0` when `entity` is `nullptr`.
    size_t ecs_get_entity_id(const entity* entity);

    //--------------------------------------------------------------
    /// @brief Adds a component to an entity through the C-style helper API.
    /// @tparam t_component_type Component type to add.
    /// @param world World that stores the component.
    /// @param entity Entity that receives the component.
    /// @param component Component value to copy.
    /// @return `true` when the component was added; otherwise `false`.
    template <typename t_component_type>
    bool ecs_world_add_component(world* world, const entity* entity, const t_component_type* component)
    {
        if (world == nullptr || entity == nullptr || component == nullptr)
        {
            return false;
        }

        return world->add_component<t_component_type>(*entity, *component);
    }

    //--------------------------------------------------------------
    /// @brief Removes a component from an entity through the C-style helper API.
    /// @tparam t_component_type Component type to remove.
    /// @param world World that stores the component.
    /// @param entity Entity to modify.
    /// @return `true` when the component was removed; otherwise `false`.
    template <typename t_component_type>
    bool ecs_world_remove_component(world* world, const entity* entity)
    {
        if (world == nullptr || entity == nullptr)
        {
            return false;
        }

        return world->remove_component<t_component_type>(*entity);
    }

    //--------------------------------------------------------------
    /// @brief Returns a mutable component pointer through the C-style helper API.
    /// @tparam t_component_type Component type to fetch.
    /// @param world World that stores the component.
    /// @param entity Entity to inspect.
    /// @return Pointer to the component, or `nullptr` when unavailable.
    template <typename t_component_type>
    t_component_type* ecs_world_get_component(world* world, const entity* entity)
    {
        if (world == nullptr || entity == nullptr)
        {
            return nullptr;
        }

        return world->get_component<t_component_type>(*entity);
    }

    //--------------------------------------------------------------
    /// @brief Returns a read-only component pointer through the C-style helper API.
    /// @tparam t_component_type Component type to fetch.
    /// @param world World that stores the component.
    /// @param entity Entity to inspect.
    /// @return Pointer to the component, or `nullptr` when unavailable.
    template <typename t_component_type>
    const t_component_type* ecs_world_get_component_const(const world* world, const entity* entity)
    {
        if (world == nullptr || entity == nullptr)
        {
            return nullptr;
        }

        return world->get_component<t_component_type>(*entity);
    }

    //--------------------------------------------------------------
    /// @brief Checks whether an entity owns a component through the C-style helper API.
    /// @tparam t_component_type Component type to test.
    /// @param world World that stores the component.
    /// @param entity Entity to inspect.
    /// @return `true` when the component exists; otherwise `false`.
    template <typename t_component_type>
    bool ecs_world_has_component(const world* world, const entity* entity)
    {
        if (world == nullptr || entity == nullptr)
        {
            return false;
        }

        return world->has_component<t_component_type>(*entity);
    }

    //--------------------------------------------------------------
    /// @brief Adds a component through the entity helper API.
    /// @tparam t_component_type Component type to add.
    /// @param entity Entity to modify.
    /// @param component Component value to copy.
    /// @return No return value.
    template <typename t_component_type>
    void ecs_entity_add_component(entity* entity, const t_component_type* component)
    {
        if (entity == nullptr || component == nullptr)
        {
            return;
        }

        entity->add_component<t_component_type>(*component);
    }

    //--------------------------------------------------------------
    /// @brief Removes a component through the entity helper API.
    /// @tparam t_component_type Component type to remove.
    /// @param entity Entity to modify.
    /// @return No return value.
    template <typename t_component_type>
    void ecs_entity_remove_component(entity* entity)
    {
        if (entity == nullptr)
        {
            return;
        }

        entity->remove_component<t_component_type>();
    }

    //--------------------------------------------------------------
    /// @brief Returns a mutable component pointer through the entity helper API.
    /// @tparam t_component_type Component type to fetch.
    /// @param entity Entity to inspect.
    /// @return Pointer to the component, or `nullptr` when unavailable.
    template <typename t_component_type>
    t_component_type* ecs_entity_get_component(entity* entity)
    {
        if (entity == nullptr)
        {
            return nullptr;
        }

        return entity->get_component<t_component_type>();
    }

    //--------------------------------------------------------------
    /// @brief Returns a read-only component pointer through the entity helper API.
    /// @tparam t_component_type Component type to fetch.
    /// @param entity Entity to inspect.
    /// @return Pointer to the component, or `nullptr` when unavailable.
    template <typename t_component_type>
    const t_component_type* ecs_entity_get_component_const(const entity* entity)
    {
        if (entity == nullptr)
        {
            return nullptr;
        }

        return entity->get_component<t_component_type>();
    }

    //--------------------------------------------------------------
    /// @brief Checks whether an entity owns a component through the entity helper API.
    /// @tparam t_component_type Component type to test.
    /// @param entity Entity to inspect.
    /// @return `true` when the component exists; otherwise `false`.
    template <typename t_component_type>
    bool ecs_entity_has_component(const entity* entity)
    {
        if (entity == nullptr)
        {
            return false;
        }

        return entity->has_component<t_component_type>();
    }
}
