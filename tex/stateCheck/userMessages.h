#pragma once

#include "../src/sfMessages.h"


// All the user commands for all tasks should be in one enum.
// For this program only 1 command is needed. 
// (Each task needs only 1 command & 1 Message struct, other than timer)
enum class UserCommands : int32_t
{
    STATE_DECISION = static_cast<int32_t>(SFCommands::SF_MAX_INTERNAL) + 1,
};


struct StateMessage : SFMessage
{
    // What is the decision? 'r' to restart and 's' to stop
    char decision;

    StateMessage() : SFMessage(static_cast<int32_t>(UserCommands::STATE_DECISION)) {}
    virtual ~StateMessage() {}
};
