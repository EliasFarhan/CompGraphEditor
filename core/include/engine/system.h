#pragma once

namespace core
{
    class System
    {
        public:
        virtual ~System() = default;

        virtual void Begin() = 0;
        virtual void Update(float dt) = 0;
        virtual void End() = 0;
    };
} // namespace core
