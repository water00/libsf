#pragma once

#include <cstdint>
#include <string>

enum class SFCommands: int32_t
{
    INNO_TEST = 1,
    INNO_TIMER_EXPIRED = 2,
    INNO_MAX_INTERNAL = 9
};

/*
enum class UserCommands : int32_t
{
    USER_TEST = static_cast<int32_t>(SFCommands::INNO_MAX_INTERNAL) + 1,
};
*/

struct SFMessage
{
    int32_t command;

    SFMessage(int32_t c) : command(c) {}
    // Virtual is necessary
    virtual ~SFMessage() {}
};

struct TestMessage : SFMessage
{
    // Test message
    std::string msg;

    TestMessage() : SFMessage(static_cast<int32_t>(SFCommands::INNO_TEST)) {}
    virtual ~TestMessage() {}
};

struct TimerMessage : SFMessage
{
    // ID of timer thet expired
    int32_t timerID;

    TimerMessage() : SFMessage(static_cast<int32_t>(SFCommands::INNO_TIMER_EXPIRED)) {}
    virtual ~TimerMessage() {}
};