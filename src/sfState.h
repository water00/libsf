#pragma once

#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <list>
#include <map>
#include "osRelated.h"
#include "sfDebug.h"

#define STATE_DEBUG

template <typename SFState, typename SFAction>
struct StateAction
{
    std::list<SFAction> actions;
    SFState nextState;
};

template <typename SFState, typename SFEvent, typename SFAction, typename OBJTYPE>
class SFStateMc
{
private:
    typedef StateAction<SFState, SFAction> SFStateAction;
    typedef std::map< SFEvent, SFStateAction > SFActionUnit;
    typedef std::multimap<SFState, SFActionUnit > SFStateTable;

    SFStateTable stateTable;
    SFState currState;

public:
    SFStateMc(SFState s) : currState(s)
    {
    }

    ~SFStateMc()
    {
        // Clear the elements of the state
        stateTable.clear();
    }

    SFState get_currState()
    {
        return currState;
    }

    int32_t add_eventActions(SFState s, SFEvent e, SFStateAction sa)
    {
        SFActionUnit actionUnit;

        actionUnit.insert(std::make_pair(e, sa));
        stateTable.insert(std::make_pair(s, actionUnit));

        return 0;
    }

    bool do_actions(OBJTYPE* o, SFEvent e)
    {
        bool stateChange = false;

        std::pair <typename SFStateTable::iterator, typename SFStateTable::iterator> stateUnitPair;

        stateUnitPair = stateTable.equal_range(currState);

        for (typename SFStateTable::iterator suItr = stateUnitPair.first; suItr != stateUnitPair.second; ++suItr)
        {
            typename SFActionUnit::iterator actionUnit = suItr->second.find(e);
            // If there is no action return
            if (actionUnit != suItr->second.end())
            {
                SFStateAction stateAction = (*(actionUnit)).second;
                for (typename std::list<SFAction>::iterator lItr = stateAction.actions.begin(); lItr != stateAction.actions.end(); lItr++)
                {
                    (o->*(*lItr))();
                }
                // Set current State
                if (currState != stateAction.nextState)
                {
                    currState = stateAction.nextState;
                    stateChange = true;
                }
                break;
            }
        }
        return stateChange;
    }
};

