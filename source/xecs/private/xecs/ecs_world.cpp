#include "xecs/ecs_world.h"
#include "xecs/ecs_entity.h"

namespace xecs
{
    //------------------------------------------------------------------
    static size_t allocate_entity_id()
    {
        static std::atomic<size_t> counter{0};
        return counter.fetch_add(1, std::memory_order_relaxed);
    }

    //------------------------------------------------------------------
    size_t get_entity_id(const entity& entity)
    {
        return entity.id();
    }

    //------------------------------------------------------------------
    size_t world::create_entity()
    {
        return allocate_entity_id();
    }
}