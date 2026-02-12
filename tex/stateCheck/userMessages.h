#pragma once

#include "sfMessages.h"


struct StateMessage : SFMessage
{
    // What is the decision? 'r' to restart and 's' to stop
    char decision;

    StateMessage() = default;
    virtual ~StateMessage() {}
};
