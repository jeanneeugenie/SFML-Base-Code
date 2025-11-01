#pragma once

#include "IWorkerAction.h"
#include "IETThread.h"

class IFinishedTask {
public:
	virtual void OnFinishedTask(int id) = 0;

};