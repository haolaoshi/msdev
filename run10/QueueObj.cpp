#include "SynchObj.h"

DWORD QueueGet(QUEUE_OBJECT* q, PVOID msg, DWORD mSize, DWORD maxWait)
{
	DWORD TotalWaitTime = 0;
	BOOL TimeOut = FALSE;

	WaitForSingleObject(q->qGuard, INFINITE);
	if (q->msgArray == NULL) return 1;

	while (QueueEmpty(q) && !TimeOut) {
		ReleaseMutex(q->qGuard);
		WaitForSingleObject(q->qNe, CV_TIMEOUT);
		if (maxWait != INFINITE) {
			TotalWaitTime += CV_TIMEOUT;
			TimeOut = (TotalWaitTime > maxWait);
		}
		WaitForSingleObject(q->qGuard, INFINITE);
	}
	if (!TimeOut) QueueRemove(q, msg, mSize);
	PulseEvent(q->qNf);
	ReleaseMutex(q->qGuard);

	return TimeOut ? WAIT_TIMEOUT : 0;
}

DWORD QueuePut(QUEUE_OBJECT* q, PVOID msg, DWORD mSize, DWORD maxWait)
{
	DWORD TotalWaitTime = 0;
	BOOL TimeOut = FALSE;

	WaitForSingleObject(q->qGuard, INFINITE);
	if (q->msgArray == NULL) return 1;

	while (QueueFull(q) && !TimeOut) {
		ReleaseMutex(q->qGuard);
		WaitForSingleObject(q->qNf, CV_TIMEOUT);
		if (maxWait != INFINITE) {
			TotalWaitTime += CV_TIMEOUT;
			TimeOut = (TotalWaitTime > maxWait);
		}
		WaitForSingleObject(q->qGuard, INFINITE);
	}
	if (!TimeOut) QueueInsert(q, msg, mSize);
	PulseEvent(q->qNe);
	ReleaseMutex(q->qGuard);

	return TimeOut ? WAIT_TIMEOUT : 0;
}

DWORD QueueInitialize(QUEUE_OBJECT* q, DWORD mSize, DWORD nMsgs)
{
	if ((q->msgArray = calloc(nMsgs, mSize)) == NULL)  return 1;
	q->qFirst = q->qLast = 0;
	q->qSize = nMsgs;
	q->qGuard = CreateMutex(NULL, FALSE, NULL);
	q->qNe = CreateEvent(NULL, TRUE, FALSE, NULL);
	q->qNf = CreateEvent(NULL, TRUE, FALSE, NULL);
	return 0;
}

DWORD QueueDestroy(QUEUE_OBJECT* q)
{
	WaitForSingleObject(q->qGuard, INFINITE);
	free(q->msgArray);
	q->msgArray = NULL;
	CloseHandle(q->qNe);
	CloseHandle(q->qNf);
	ReleaseMutex(q->qGuard);
	CloseHandle(q->qGuard);

	return 0;
}


DWORD QueueEmpty(QUEUE_OBJECT* q)
{
	return (q->qFirst == q->qLast);
}

DWORD QueueFull(QUEUE_OBJECT* q)
{
	return ((q->qFirst - q->qLast) == 1 || (q->qLast == q->qSize - 1 && q->qFirst == 0));
}

DWORD QueueRemove(QUEUE_OBJECT* q, PVOID msg, DWORD mSize)
{
	char* pm;
	if (QueueEmpty(q)) return 1;
	pm = q->msgArray;
	memcpy(msg, pm + (q->qFirst * mSize), mSize);
	q->qFirst = ((q->qFirst + 1) % q->qSize);
	return 0;
}

DWORD QueueInsert(QUEUE_OBJECT* q, PVOID msg, DWORD mSize)
{
	char* pm;
	if (QueueFull(q)) return 1;
	pm = q->msgArray;
	memcpy(pm + (q->qLast * mSize), msg, mSize);
	q->qLast = ((q->qLast + 1) % q->qSize);

	return 0;
}