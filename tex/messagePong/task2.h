#pragma once

#include <stdint.h>
#include <string>
#include <list>
#include "sfTask.h"

class Task2 : public SFTask
{
private:
	int32_t cnt;

public:
	Task2();
	virtual ~Task2();

	virtual void processFn();
};

