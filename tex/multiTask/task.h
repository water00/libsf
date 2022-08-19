#pragma once

#include <stdint.h>
#include <string>
#include <list>
#include "../../src/sfTask.h"


class Task : public SFTask
{
private:
	int32_t taskNum;
	int32_t numMessages;
	int32_t cnt;

public:
	Task(int32_t pNo, int32_t numMsg);
	virtual ~Task();

	virtual void processFn();
};
