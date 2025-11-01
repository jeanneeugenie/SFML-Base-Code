#pragma once

class IExecutionEvent {
public:
	virtual void OnFinishedExecution() = 0;
};