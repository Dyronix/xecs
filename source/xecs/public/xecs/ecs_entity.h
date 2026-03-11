#pragma once

#include "xecs/ecs_world.h"

#include <cstdint>

namespace xecs
{
    class entity
    {
    public:       
        size_t id() const { return m_id; }

        template<typename TComponentType>
        bool add_component(const TComponentType& component)
        {
            return m_world->add_component<TComponentType>(*this, component);
        }
        template<typename TComponentType>
        bool remove_component()
        {
            return m_world->remove_component<TComponentType>(*this);
        }

        template<typename TComponentType>
        TComponentType* get_component()
        {
            return m_world->get_component<TComponentType>(*this);
        }
        template<typename TComponentType>
        const TComponentType* get_component() const
        {
            return m_world->get_component<TComponentType>(*this);
        }

        template<typename TComponentType>
        bool has_component() const
        {
            return m_world->has_component<TComponentType>(*this);
        }

    private:
        friend class world;

        entity(world* world)
            : m_world(world)
            , m_id(world != nullptr ? world->create_entity() : 0) 
        {
            
        }

    private:
        world* m_world;
        size_t m_id;
    };    
} // namespace xecs
