#pragma once

#include "EventBase.h"

namespace Cosmos::Platform
{

    class WindowCloseEvent : public EventBase
    {
    public:

        // constructor
        WindowCloseEvent() : EventBase("Window Close", EventType::WindowClose) {}

        // destructor
        ~WindowCloseEvent() = default;

    private:

    };

    class WindowResizeEvent : public EventBase
    {
    public:

        // constructor
        WindowResizeEvent(int width, int height) : EventBase("Window Resize", EventType::WindowResize), mWidth(width), mHeight(height) {}

        // destructor
        ~WindowResizeEvent() = default;

        // returns the new width 
        inline int GetWidth() const { return mWidth; }

        // returns the new height
        inline int GetHeight() const { return mHeight; }

    private:

        int mWidth;
        int mHeight;
    };
}