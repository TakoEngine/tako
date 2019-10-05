#pragma once
#include "Event.hpp"
#include <array>

namespace tako
{
    class Input : public IEventHandler
    {
    public:
        virtual void HandleEvent(Event& evt) override;
        void Update();
        bool GetKey(Key key);
        bool GetKeyDown(Key key);
        bool GetKeyUp(Key key);
    private:
        std::array<KeyStatus, static_cast<size_t>(Key::Unknown)> activeKeys;
        std::array<KeyStatus, static_cast<size_t>(Key::Unknown)> keys;
        std::array<KeyStatus, static_cast<size_t>(Key::Unknown)> prevKeys;
    };
}
