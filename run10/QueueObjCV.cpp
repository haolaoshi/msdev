/*10-6 The Queue Management Function */

#include "SynchObj.h"

DWORD QueueGet(QUEUE_OBJECT* q, PVOID msg, DWORD mSize, DWORD maxWait)
{
	EnterCriticalSection(&q->qGuard);
	while (QueueEmpty(q))
	{
		SleepConditionVariableCS(&q->qNe, &q->qGuard, INFINITE);
	}

	QueueRemove(q, msg, mSize);

	WakeConditionVariable(&q->qNf);
	LeaveCriticalSection(&q->qGuard);
	return 0;
}

DWORD QueuePut(QUEUE_OBJECT* q, PVOID msg, DWORD mSize, DWORD maxWait)
{
	EnterCriticalSection(&q->qGuard);
	while (QueueFull(q)) {
		SleepConditionVariableCS(&q->qNf, &q->qGuard, INFINITE);
	}
	QueueInsert(q, msg, mSize);
	WakeConditionVariable(&q->qNe);
	LeaveCriticalSection(&q->qGuard);
	return 0;
}

DWORD QueueIntialize(QUEUE_OBJECT* q, DWORD mSize, DWORD nMsgs)
{
	q->qFirst = q->qLast = 0;
	q->qSize = nMsgs;

	InitializeCriticalSection(&q->qGuard);
	InitializeConditionVariable(&q->qNe);
	InitializeConditionVariable(&q->qNf);
	if ((q->msgArray = calloc(nMsgs, mSize)) == NULL) return 1;
	return 0;
}

DWORD QueueDestroy(QUEUE_OBJECT* q)
{
	EnterCriticalSection(&q->qGuard);
	free(q->msgArray);
	LeaveCriticalSection(&(q->qGuard));
	DeleteCriticalSection(&(q->qGuard));
	return 0;
}

DWORD QueueEmpty(QUEUE_OBJECT* q)
{
	return (q->qFirst == q->qLast);
}

DWORD QueueFull(QUEUE_OBJECT* q)
{
	return ((q->qFirst - q->qLast) == 1) || (q->qLast == q->qSize - 1 && q->qFirst == 0));
}

DWORD QueueRemove(QUEUE_OBJECT* q, PVOID msg, DWORD mSize)
{
	char* pm;
	if (QueueEmpty(q)) return 1;
	pm = q->msgArray;
	memcpy(msg, pm + (q->qFirst + mSize), mSize);
	q->qFirst = ((q->qFirst + 1) % (q->qSize));
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