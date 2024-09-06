#pragma once

#include "EventBase.h"

namespace Cosmos::Platform
{
    class MousePressEvent : public EventBase
    {
    public:

        // constructor
        MousePressEvent(Buttoncode button) : EventBase("Mouse Press", EventType::MousePress), mButtonCode(button) {}

        // destructor
        ~MousePressEvent() = default;

        // returns the button code 
        inline Buttoncode GetButtoncode() const { return mButtonCode; }

    private:

        Buttoncode mButtonCode;
    };

    class MouseReleaseEvent : public EventBase
    {
    public:

        // constructor
        MouseReleaseEvent(Buttoncode button) : EventBase("Mouse Release", EventType::MouseRelease), mButtonCode(button) {}

        // destructor
        ~MouseReleaseEvent() = default;

        // returns the button code 
        inline Buttoncode GetButtoncode() const { return mButtonCode; }

    private:

        Buttoncode mButtonCode;
    };

    class MouseWheelEvent : public EventBase
    {
    public:

        // constructor
        MouseWheelEvent(int delta) : EventBase("Mouse Wheel", EventType::MouseWheel), mDelta(delta) {}

        // destructor
        ~MouseWheelEvent() = default;

        // returns the delta move
        inline int GetDelta() const { return mDelta; }

    private:

        int mDelta;
    };

    class MouseMoveEvent : public EventBase
    {
    public:

        // constructor
        MouseMoveEvent(int xOffset, int yOffset) : EventBase("Mouse Move", EventType::MouseMove), mXOffset(xOffset), mYOffset(yOffset) {}

        // destructor
        ~MouseMoveEvent() = default;

        // returns the new x coodirnate
        inline int GetXOffset() const { return mXOffset; }

        // returns the new y coordinates
        inline int GetYOffset() const { return mYOffset; }

    private:

        int mXOffset;
        int mYOffset;
    };
}