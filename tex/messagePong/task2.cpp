#include <iostream>
#include <string>
#include <sstream>
#include "task1.h"
#include "task2.h"
#include "sfMessages.h"

extern Task1 th1;

Task2::Task2()
{
	cnt = 0;

	// Create sock
	create_sock();
	
	// Create a struct with all the process fns and 
	// related socket
	ProcessStruct<PROCFN> pStruct;
	pStruct.sock = socks[0];
	pStruct.processFn = &SFTask::processFn;
	pStruct.processObj = this;

	SFTask::sfThread->add_process(pStruct);

}

Task2::~Task2()
{
}

void Task2::processFn()
{
	TestMessage gMsg;

	while (getMessage(gMsg))
	{
		std::cout << "Th2, Msg From Th1: " << gMsg.msg << std::endl;
	}

	std::stringstream th2Data;
	th2Data << "This is my message with Count = " << cnt++;

	
	TestMessage sMsg;
	sMsg.msg = th2Data.str();

	// Add the message to task1 data queue
	th1.addMessage(sMsg);
}
