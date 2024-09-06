#pragma once

namespace Cosmos::Platform
{
    enum EventType : int
    {
        Undefined = -1,

        // input events
        KeyboardPress,
        KeyboardRelease,
        MousePress,
        MouseRelease,
        MouseWheel,
        MouseMove,

        // window events
        WindowClose,
        WindowResize,

        EventMax
    };

    class EventBase
    {
    public:

        // constructor
        EventBase(const char* name = "Event", EventType type = EventType::Undefined) : mName(name), mType(type) { }

        // destructor
        virtual ~EventBase() = default;

        // returns the name
        inline const char* GetName() { return mName; }

        // returns the event type
        inline EventType GetType() const { return mType; }

    private:

        const char* mName;
        EventType mType;
    };
}