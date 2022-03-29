#include "FThread.h"

void* _threadHandle[FThread::eTH_COUNT];
bool _threadAliveFlag[FThread::eTH_COUNT];
unsigned int _threadID[FThread::eTH_COUNT];
unsigned long _interval[FThread::eTH_COUNT];

FThread::FThread()
{
	_lib = Fmhha();

	for (int i = 0; i < eTH_COUNT; i++)
	{
		_threadAliveFlag[i] = false;
		_interval[i] = 10;
	}
}
FThread::~FThread()
{
}



static unsigned int __stdcall ThreadFunc0(void* pParam)
{
	FThread* pthis = static_cast<FThread*>(pParam);
	pthis->PreThread0();
	int index = pthis->eTH0;
	while (_threadAliveFlag[index])
	{
		Sleep(_interval[index]);
		if (pthis->ProcThread0() == false)
		{
			break;
		}
	}
	pthis->PostThread0();
	CloseHandle(_threadHandle[index]);

	return 0;
}
static unsigned int __stdcall ThreadFunc1(void* pParam)
{
	FThread* pthis = static_cast<FThread*>(pParam);
	pthis->PreThread1();
	int index = pthis->eTH1;
	while (_threadAliveFlag[index])
	{
		Sleep(_interval[index]);
		if (pthis->ProcThread1() == false)
		{
			break;
		}
	}
	pthis->PostThread1();
	CloseHandle(_threadHandle[index]);

	return 0;
}
static unsigned int __stdcall ThreadFunc2(void* pParam)
{
	FThread* pthis = static_cast<FThread*>(pParam);
	pthis->PreThread2();
	int index = pthis->eTH2;
	while (_threadAliveFlag[index])
	{
		Sleep(_interval[index]);
		if (pthis->ProcThread2() == false)
		{
			break;
		}
	}
	pthis->PostThread2();
	CloseHandle(_threadHandle[index]);

	return 0;
}
static unsigned int __stdcall ThreadFunc3(void* pParam)
{
	FThread* pthis = static_cast<FThread*>(pParam);
	pthis->PreThread3();
	int index = pthis->eTH3;
	while (_threadAliveFlag[index])
	{
		Sleep(_interval[index]);
		if (pthis->ProcThread3() == false)
		{
			break;
		}
	}
	pthis->PostThread3();
	CloseHandle(_threadHandle[index]);

	return 0;
}
static unsigned int __stdcall ThreadFunc4(void* pParam)
{
	FThread* pthis = static_cast<FThread*>(pParam);
	pthis->PreThread4();
	int index = pthis->eTH4;
	while (_threadAliveFlag[index])
	{
		Sleep(_interval[index]);
		if (pthis->ProcThread4() == false)
		{
			break;
		}
	}
	pthis->PostThread4();
	CloseHandle(_threadHandle[index]);

	return 0;
}
static unsigned int __stdcall ThreadFunc5(void* pParam)
{
	FThread* pthis = static_cast<FThread*>(pParam);
	pthis->PreThread5();
	int index = pthis->eTH5;
	while (_threadAliveFlag[index])
	{
		Sleep(_interval[index]);
		if (pthis->ProcThread5() == false)
		{
			break;
		}
	}
	pthis->PostThread5();
	CloseHandle(_threadHandle[index]);

	return 0;
}
static unsigned int __stdcall ThreadFunc6(void* pParam)
{
	FThread* pthis = static_cast<FThread*>(pParam);
	pthis->PreThread6();
	int index = pthis->eTH6;
	while (_threadAliveFlag[index])
	{
		Sleep(_interval[index]);
		if (pthis->ProcThread6() == false)
		{
			break;
		}
	}
	pthis->PostThread6();
	CloseHandle(_threadHandle[index]);

	return 0;
}
static unsigned int __stdcall ThreadFunc7(void* pParam)
{
	FThread* pthis = static_cast<FThread*>(pParam);
	pthis->PreThread7();
	int index = pthis->eTH7;
	while (_threadAliveFlag[index])
	{
		Sleep(_interval[index]);
		if (pthis->ProcThread7() == false)
		{
			break;
		}
	}
	pthis->PostThread7();
	CloseHandle(_threadHandle[index]);

	return 0;
}
static unsigned int __stdcall ThreadFunc8(void* pParam)
{
	FThread* pthis = static_cast<FThread*>(pParam);
	pthis->PreThread8();
	int index = pthis->eTH8;
	while (_threadAliveFlag[index])
	{
		Sleep(_interval[index]);
		if (pthis->ProcThread8() == false)
		{
			break;
		}
	}
	pthis->PostThread8();
	CloseHandle(_threadHandle[index]);

	return 0;
}
static unsigned int __stdcall ThreadFunc9(void* pParam)
{
	FThread* pthis = static_cast<FThread*>(pParam);
	pthis->PreThread9();
	int index = pthis->eTH9;
	while (_threadAliveFlag[index])
	{
		Sleep(_interval[index]);
		if (pthis->ProcThread9() == false)
		{
			break;
		}
	}
	pthis->PostThread9();
	CloseHandle(_threadHandle[index]);

	return 0;
}

bool FThread::CreateThread(int index)
{
	if (_lib.LibraryPermit() == false)
	{
		return false;
	}

	if (index < eTH0 || index >= eTH_COUNT)
	{
		return false;
	}

	if (_threadAliveFlag[index] == false)
	{
		_threadAliveFlag[index] = true;
		switch (index)
		{
		case eTH0:	_threadHandle[index] = (void*)_beginthreadex(NULL, 0, ThreadFunc0, this, 0, &(_threadID[index]));	break;
		case eTH1:	_threadHandle[index] = (void*)_beginthreadex(NULL, 0, ThreadFunc1, this, 0, &(_threadID[index]));	break;
		case eTH2:	_threadHandle[index] = (void*)_beginthreadex(NULL, 0, ThreadFunc2, this, 0, &(_threadID[index]));	break;
		case eTH3:	_threadHandle[index] = (void*)_beginthreadex(NULL, 0, ThreadFunc3, this, 0, &(_threadID[index]));	break;
		case eTH4:	_threadHandle[index] = (void*)_beginthreadex(NULL, 0, ThreadFunc4, this, 0, &(_threadID[index]));	break;
		case eTH5:	_threadHandle[index] = (void*)_beginthreadex(NULL, 0, ThreadFunc5, this, 0, &(_threadID[index]));	break;
		case eTH6:	_threadHandle[index] = (void*)_beginthreadex(NULL, 0, ThreadFunc6, this, 0, &(_threadID[index]));	break;
		case eTH7:	_threadHandle[index] = (void*)_beginthreadex(NULL, 0, ThreadFunc7, this, 0, &(_threadID[index]));	break;
		case eTH8:	_threadHandle[index] = (void*)_beginthreadex(NULL, 0, ThreadFunc8, this, 0, &(_threadID[index]));	break;
		case eTH9:	_threadHandle[index] = (void*)_beginthreadex(NULL, 0, ThreadFunc9, this, 0, &(_threadID[index]));	break;
		}

		if (_threadHandle[index] < 0)
		{
			_threadAliveFlag[index] = false;
			return false;
		}
	}

	return true;
}
bool FThread::CloseThread(int index)
{
	if (_lib.LibraryPermit() == false)
	{
		return false;
	}

	if (index < eTH0 || index > eTH_COUNT)
	{
		return false;
	}

	if (index == eTH_COUNT)
	{
		for (int i = 0; i < eTH_COUNT; i++)
		{
			if (_threadAliveFlag[i] == true)
			{
				_threadAliveFlag[i] = false;
			}
		}

		return true;
	}

	if (_threadAliveFlag[index] == true)
	{
		_threadAliveFlag[index] = false;
	}

	return true;
}
unsigned long FThread::WaitThreadTerminate(int index, int timeout)
{
	if (_lib.LibraryPermit() == false)
	{
		return WAIT_FAILED;
	}

	if (index < eTH0 || index >= eTH_COUNT)
	{
		return WAIT_FAILED;
	}

	return WaitForSingleObject(_threadHandle[index], timeout);
}

void* FThread::GetThreadHandle(int index)
{
	if (_lib.LibraryPermit() == false)
	{
		return _threadHandle[9];
	}

	if (index < eTH0 || index >= eTH_COUNT)
	{
		return _threadHandle[9];
	}

	return _threadHandle[index];
}
bool FThread::IsThreadAlive(int index)
{
	if (_lib.LibraryPermit() == false)
	{
		return false;
	}

	if (index < eTH0 || index >= eTH_COUNT)
	{
		return false;
	}

	return _threadAliveFlag[index];
}
unsigned int FThread::GetThreadID(int index)
{
	if (_lib.LibraryPermit() == false)
	{
		return 0;
	}

	if (index < eTH0 || index >= eTH_COUNT)
	{
		return 0;
	}

	return _threadID[index];
}
void FThread::SetInterval(int index, unsigned long interval)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	if (index < eTH0 || index >= eTH_COUNT)
	{
		return;
	}

	_interval[index] = interval;
}
unsigned long FThread::GetInterval(int index)
{
	if (_lib.LibraryPermit() == false)
	{
		return 0;
	}

	if (index < eTH0 || index >= eTH_COUNT)
	{
		return 0;
	}

	return _interval[index];
}