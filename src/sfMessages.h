#pragma once

#include <cstdint>
#include <string>
#include <any>

enum class SFType: int32_t
{
    SFTYPE_TIMER = 1,
    SFTYPE_USER = 2,
};

struct SFMessage
{
    SFType type;

    SFMessage(SFType t = SFType::SFTYPE_USER) : type(t) {}
    // Virtual is necessary
    virtual ~SFMessage() {}
    // Get type
    virtual SFType getType() { return type;}
};

struct TestMessage : SFMessage
{
    // Test message
    std::string msg;

    TestMessage() : SFMessage() {}
    virtual ~TestMessage() {}
};

struct TimerMessage : SFMessage
{
    // ID of timer that expired
    int32_t timerID;
    std::any userData;

    TimerMessage() : 
        SFMessage(SFType::SFTYPE_TIMER),
        timerID(0) 
        {
        }
    virtual ~TimerMessage() {}
};