#include "SynchObj.h"

DWORD CreateThresholdBarrier(THB_OBJECT* pThb, DWORD bValue)
{
	THB_OBJECT objThb;
	objThb = (THB_OBJECT)malloc(sizeof(THRESHOLD_BARRIER));
	if (objThb == NULL) return SYNCH_OBJ_MODEM;

	objThb->bGuarg = CreateMutex(NULL, FALSE, NULL);
	if (objThb->bGuarg == NULL) return SYNCH_OBJ_CREATE_FAILURE;

	objThb->bEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (objThb->bEvent == NULL) return SYNCH_OBJ_CREATE_FAILURE;

	objThb->bThreshold = bValue;
	objThb->bCount = 0;

	*pThb = objThb;
	return 0;
}

DWORD WaitThresholdBarrier(THB_OBJECT thb)
{
	WaitForSingleObject(thb->bGuarg, INFINITE);
	thb->bCount++;
	while (thb->bCount < thb->bThreshold) {
		SignalObjectAndWait(thb->bGuarg, thb->bEvent, INFINITE, FALSE);
		WaitForSingleObject(thb->bGuarg, INFINITE);
	}
	SetEvent(thb->bEvent);
	ReleaseMutex(thb->bGuarg);
	return 0;
}

DWORD CloseThresholdBarrier(THB_OBJECT thb)
{
	WaitForSingleObject(thb->bGuarg, INFINITE);
	if (thb->bCount < thb->bThreshold) {
		ReleaseMutex(thb->bGuarg);
		return SYNCH_OBJ_BUSY;
	}
	ReleaseMutex(thb->bGuarg);
	CloseHandle(thb->bEvent);
	CloseHandle(thb->bGuarg);
	free(thb);
	return 0;
}