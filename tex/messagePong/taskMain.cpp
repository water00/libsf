#include <iostream>
#include <string>
#include "task1.h"
#include "task2.h"

using namespace std;

Task1	th1;
Task2 th2;

// Declare static fn of sfTask here. 
SFThread<PROCFN>* SFTask::sfThread;
int32_t SFTask::taskCount = 0;

int main()
{

	th1.start_trans();

	while(1)
		sleep(100);
}

