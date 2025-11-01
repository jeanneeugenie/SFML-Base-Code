#pragma once

class IWorkerAction {

//Threads' run function
public:
	virtual void OnStartTask() = 0;

};