#pragma once

#include "xecs/ecs_entity.h"
#include "xecs/ecs_world.h"

namespace xecs
{
    //--------------------------------------------------------------
    world* ecs_world_create();

    //--------------------------------------------------------------
    void ecs_world_destroy(world* world);

    //--------------------------------------------------------------
    size_t ecs_world_create_entity(world* world);

    //--------------------------------------------------------------
    size_t ecs_entity_id(const entity* entity);

    //--------------------------------------------------------------
    size_t ecs_get_entity_id(const entity* entity);

    //--------------------------------------------------------------
    template <typename TComponentType>
    bool ecs_world_add_component(world* world, const entity* entity, const TComponentType* component)
    {
        if (world == nullptr || entity == nullptr || component == nullptr)
        {
            return false;
        }

        return world->add_component<TComponentType>(*entity, *component);
    }

    //--------------------------------------------------------------
    template <typename TComponentType>
    bool ecs_world_remove_component(world* world, const entity* entity)
    {
        if (world == nullptr || entity == nullptr)
        {
            return false;
        }

        return world->remove_component<TComponentType>(*entity);
    }

    //--------------------------------------------------------------
    template <typename TComponentType>
    TComponentType* ecs_world_get_component(world* world, const entity* entity)
    {
        if (world == nullptr || entity == nullptr)
        {
            return nullptr;
        }

        return world->get_component<TComponentType>(*entity);
    }

    //--------------------------------------------------------------
    template <typename TComponentType>
    const TComponentType* ecs_world_get_component_const(const world* world, const entity* entity)
    {
        if (world == nullptr || entity == nullptr)
        {
            return nullptr;
        }

        return world->get_component<TComponentType>(*entity);
    }

    //--------------------------------------------------------------
    template <typename TComponentType>
    bool ecs_world_has_component(const world* world, const entity* entity)
    {
        if (world == nullptr || entity == nullptr)
        {
            return false;
        }

        return world->has_component<TComponentType>(*entity);
    }

    //--------------------------------------------------------------
    template <typename TComponentType>
    void ecs_entity_add_component(entity* entity, const TComponentType* component)
    {
        if (entity == nullptr || component == nullptr)
        {
            return;
        }

        entity->add_component<TComponentType>(*component);
    }

    //--------------------------------------------------------------
    template <typename TComponentType>
    void ecs_entity_remove_component(entity* entity)
    {
        if (entity == nullptr)
        {
            return;
        }

        entity->remove_component<TComponentType>();
    }

    //--------------------------------------------------------------
    template <typename TComponentType>
    TComponentType* ecs_entity_get_component(entity* entity)
    {
        if (entity == nullptr)
        {
            return nullptr;
        }

        return &entity->get_component<TComponentType>();
    }

    //--------------------------------------------------------------
    template <typename TComponentType>
    const TComponentType* ecs_entity_get_component_const(const entity* entity)
    {
        if (entity == nullptr)
        {
            return nullptr;
        }

        return &entity->get_component<TComponentType>();
    }

    //--------------------------------------------------------------
    template <typename TComponentType>
    bool ecs_entity_has_component(const entity* entity)
    {
        if (entity == nullptr)
        {
            return false;
        }

        return entity->has_component<TComponentType>();
    }
}
