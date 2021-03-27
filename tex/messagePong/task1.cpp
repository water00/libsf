#include <iostream>
#include <string>
#include <sstream>
#include "task2.h"
#include "task1.h"
#include "task2.h"
#include "sfMessages.h"

extern Task2 th2;


Task1::Task1()
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

Task1::~Task1()
{
}

void Task1::processFn()
{
	TestMessage gMsg;

	while (getMessage(gMsg))
	{
		std::cout << "Th1, Msg From Th2: " << gMsg.msg << std::endl;
	}

	std::stringstream th1Data;
	th1Data << "This is my message with Count = " << cnt++;
	
	TestMessage sMsg;
	sMsg.msg = th1Data.str();

	// Add the message to task2 data queue
	th2.addMessage(sMsg);
}

void Task1::start_trans()
{
	std::string th1Data = "This is my initial message";
	th1Data += cnt++;

	TestMessage sMsg;
	sMsg.msg = th1Data;

	// Add the message to task2 data queue
	th2.addMessage(sMsg);
}
