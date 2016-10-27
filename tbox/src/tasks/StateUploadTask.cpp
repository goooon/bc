#include "./StateUploadTask.h"
Task* StateUploadTask::Create()
{
	return bc_new UploadStateTask();
}

void StateUploadTask::doTask()
{
	reqSendState();
}

void StateUploadTask::reqSendState()
{

}

