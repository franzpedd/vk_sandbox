#pragma once

#include "EventBase.h"

namespace Cosmos::Platform
{
    class KeyboardPressEvent : public EventBase
    {
    public:

        // constructor
        KeyboardPressEvent(Keycode key, Keymod mod = KEYMOD_NONE) : EventBase("Keyboard Press", EventType::KeyboardPress), mKeycode(key), mKeymod(mod) {}

        // destructor
        ~KeyboardPressEvent() = default;

        // returns the key code 
        inline Keycode GetKeycode() const { return mKeycode; }

        // returns the key modifier
        inline Keymod GetKeymod() const { return mKeymod; }

    private:

        Keycode mKeycode;
        Keymod mKeymod;
    };

    class KeyboardReleaseEvent : public EventBase
    {
    public:

        // constructor
        KeyboardReleaseEvent(Keycode key, Keymod mod = KEYMOD_NONE) : EventBase("Keyboard Release", EventType::KeyboardRelease), mKeycode(key), mKeymod(mod) {}

        // destructor
        ~KeyboardReleaseEvent() = default;

        // returns the key code 
        inline Keycode GetKeycode() const { return mKeycode; }

        // returns the key modifier
        inline Keymod GetKeymod() const { return mKeymod; }

    private:

        Keycode mKeycode;
        Keymod mKeymod;
    };
}