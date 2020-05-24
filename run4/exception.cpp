#include <Windows.h>
#include <float.h>
#include <tchar.h>

#include <stdio.h>


DWORD Filter(LPEXCEPTION_POINTERS,LPDWORD);
VOID ReportException(LPCTSTR userMessage,DWORD exceptionCode);
double x = 1.0, y = 1.0;
int _tmain(int argc,LPTSTR argv[])
{
	DWORD  eCategory,i =0, ix, iy = 0;
	LPDWORD pNull = NULL;
	BOOL done = FALSE;
	DWORD fpOld , fpNew;
	fpOld = _controlfp(0,0);
	fpNew = fpOld & ~(ERROR_EOM_OVERFLOW|RPC_S_FP_UNDERFLOW | NUMPRS_INEXACT|RPC_S_ZERO_DIVIDE|STATUS_FLOAT_DENORMAL_OPERAND|GMEM_INVALID_HANDLE);
	_controlfp(fpNew,MCW_EM);
	while(!done) __try{
		_tprintf(_T("Enter exception type:"));
		_tprintf(_T("1,mem. 2,int. 3,flt. 4,User. 5,leave"));
		_tscanf(_T("%d"),&i);
		__try{
			switch(i){
			case 1:
				ix = *pNull; *pNull = 5; break;
			case 2:
				ix = ix / iy; break;
			case 3:
				x = x / y ;
				_tprintf(_T("x = %20e\n"),x);break;
			case 4:
				ReportException(_T("User Exception"),1);break;
			case 5:
				__leave;
			default:
				done = TRUE;
			}
		}
		__except(Filter(GetExceptionInformation(),&eCategory))
		{
			switch(eCategory){
			case 0:
				_tprintf(_T("unknown exception\n"));continue;
			case 2:
				_tprintf(_T("Integer Eception\n"));break;
			case 3:
				_tprintf(_T("Floating point Exception\n"));break;
			case 10:
				_tprintf(_T("User Exception\n"));break;
			default:
				_tprintf(_T("unknow excetion\n"));break;
			}
			_tprintf(_T("End of handler\n"));
		}
	}
	__finally{
		_tprintf(_T("Abnormal Termination ? %d\n"),AbnormalTermination());

	}
	_controlfp(fpOld,0xFFFFFFFF);
	return 0;
}

static DWORD Filter(LPEXCEPTION_POINTERS pExp,LPDWORD eCategory)
{

	DWORD exCode ,readWrite,virtAddr;
	exCode = pExp->ExceptionRecord->ExceptionCode;
	_tprintf(_T("Filter exCode : %x\n"),exCode);
	if((0x20000000 & exCode ) != 0) {
		*eCategory = 10;
		return EXCEPTION_EXECUTE_HANDLER;
	}
	switch(exCode){
	case EXCEPTION_ACCESS_VIOLATION:
		readWrite = pExp->ExceptionRecord->ExceptionInformation[0];
		virtAddr = pExp->ExceptionRecord->ExceptionInformation[1];
		_tprintf(_T("Access Violoation. Read/Write/Exec: %d. Address: %x\n"),readWrite,virtAddr);
		*eCategory = 1;
		return EXCEPTION_EXECUTE_HANDLER;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
		*eCategory = 2;
		return EXCEPTION_EXECUTE_HANDLER;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_OVERFLOW:
		_tprintf(_T("Flt Exception - large Result .\n"));
		*eCategory = 3;
		_clearfp();
		return EXCEPTION_EXECUTE_HANDLER;
	default:
		*eCategory = 0;
		return EXCEPTION_CONTINUE_EXECUTION;
	}
}