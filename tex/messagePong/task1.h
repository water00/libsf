#pragma once

#include <stdint.h>
#include <string>
#include <list>
#include "sfTask.h"


class Task1 : public SFTask
{
private:
	int32_t cnt;

public:
	Task1();
	virtual ~Task1();

	virtual void processFn();
	void start_trans();
};

