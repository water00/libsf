#include <iostream>
#include <string>
#include <sstream>
#include "task.h"
#include "sfDebug.h"
#include "sfTask.h"

extern Task* rand_process();
extern bool stopTasks;

extern SFDebug errdbg;
extern SFDebug wngdbg;
extern SFDebug ifodbg;
extern SFDebug vbsdbg;

Task::Task(int32_t pNo, int32_t numMsg)
{
	cnt = 0;
	taskNum = pNo;
	numMessages = numMsg;

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

Task::~Task()
{
}

void Task::processFn()
{
	TestMessage gMsg;

	while (getMessage(gMsg))
	{
		std::cout << "Task " << taskNum << ": " << gMsg.msg << std::endl;
	}

	std::stringstream data;
	data << "Message from Task " << taskNum << ", Count: " << cnt++;
	
	if (cnt >= numMessages)
	{
		wngdbg << "Task: " << taskNum << " Has reached the count. Stopping........" << "\n";
		stopTasks = true;
	}
	else
	{
		TestMessage sMsg;
		sMsg.msg = data.str();

		// Add the message to new random data queue. If it fails it means at least
		// one task has completed. Otherwise it wont fail in our simple example. 
		// Then stop
		if (!rand_process()->addMessage(sMsg)) 
		{
			stopTasks = true;
		}
	}
}
