#include "xecs/ecs_entity.h"

namespace xecs
{
    size_t entity_id(const entity& entity)
    {
        return entity.id();
    }
}
