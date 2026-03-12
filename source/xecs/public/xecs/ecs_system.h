#pragma once

#include <cstddef>

namespace xecs
{
    class world;

    class isystem
    {
    public:
        virtual ~isystem() = default;

        /// @brief Executes the system against a world.
        /// @param world World instance to process.
        /// @return No return value.
        virtual void execute(world& world) = 0;
    };

    class system
    {
    public:
        /// @brief Returns the registered system identifier.
        /// @return System identifier value.
        size_t id() const;

        /// @brief Checks whether the system handle references a world.
        /// @return `true` when the handle is usable; otherwise `false`.
        bool valid() const;

        /// @brief Executes the registered system in its owning world.
        /// @return No return value.
        void execute() const;

    private:
        friend class world;

        /// @brief Creates a system handle.
        /// @param world World that owns the registered system.
        /// @param id Registered system identifier.
        system(world* world, size_t id);

    private:
        world* m_world{ nullptr };
        size_t m_id{ 0 };
    };
}
