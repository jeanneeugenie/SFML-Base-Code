#include "LoadAssetThread.h"

LoadAssetThread::LoadAssetThread(int _id,
	IExecutionEvent* callback)
{
	this->id = _id;
	this->onFinished = callback;
}

void LoadAssetThread::OnStartTask()
{
	TextureManager::getInstance()->loadSingleStreamAsset(this->id);
	this->onFinished->OnFinishedExecution();
}


//925 edited out
// void LoadAssetThread::run()
//{
//	TextureManager::getInstance->LoadSingleStreamAsset(this->id);
//	this->onFinished->OnFinishedExecution();
//
//	delete this;
//}