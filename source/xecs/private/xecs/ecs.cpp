#include "xecs/ecs.h"

namespace xecs
{
    //--------------------------------------------------------------
    world* ecs_world_create()
    {
        return new world();
    }

    //--------------------------------------------------------------
    void ecs_world_destroy(world* world)
    {
        delete world;
    }

    //--------------------------------------------------------------
    size_t ecs_world_create_entity(world* world)
    {
        return world != nullptr ? world->create_entity() : 0;
    }

    //--------------------------------------------------------------
    size_t ecs_entity_id(const entity* entity)
    {
        return entity != nullptr ? entity->id() : 0;
    }

    //--------------------------------------------------------------
    size_t ecs_get_entity_id(const entity* entity)
    {
        return entity != nullptr ? get_entity_id(*entity) : 0;
    }
}
