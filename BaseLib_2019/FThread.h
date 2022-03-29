#pragma once
#include <process.h>
#include <Windows.h>
#include "lib/Fmhha.h"

class __declspec(dllexport) FThread
{
public:
	FThread();
	virtual~FThread();

	enum
	{
		eTH0		= 0,
		eTH1		,
		eTH2		,
		eTH3		,
		eTH4		,
		eTH5		,
		eTH6		,
		eTH7		,
		eTH8		,
		eTH9		,
		eTH_COUNT	,
	};
private:
	Fmhha _lib;

public:
	virtual void PreThread0() { return; }
	virtual void PreThread1() { return; }
	virtual void PreThread2() { return; }
	virtual void PreThread3() { return; }
	virtual void PreThread4() { return; }
	virtual void PreThread5() { return; }
	virtual void PreThread6() { return; }
	virtual void PreThread7() { return; }
	virtual void PreThread8() { return; }
	virtual void PreThread9() { return; }

	virtual void PostThread0() { return; }
	virtual void PostThread1() { return; }
	virtual void PostThread2() { return; }
	virtual void PostThread3() { return; }
	virtual void PostThread4() { return; }
	virtual void PostThread5() { return; }
	virtual void PostThread6() { return; }
	virtual void PostThread7() { return; }
	virtual void PostThread8() { return; }
	virtual void PostThread9() { return; }

	virtual bool ProcThread0() { return false; }
	virtual bool ProcThread1() { return false; }
	virtual bool ProcThread2() { return false; }
	virtual bool ProcThread3() { return false; }
	virtual bool ProcThread4() { return false; }
	virtual bool ProcThread5() { return false; }
	virtual bool ProcThread6() { return false; }
	virtual bool ProcThread7() { return false; }
	virtual bool ProcThread8() { return false; }
	virtual bool ProcThread9() { return false; }

	bool CreateThread(int index);
	bool CloseThread(int index = eTH_COUNT);
	unsigned long WaitThreadTerminate(int index, int timeout = 5 * 60 * 1000);

	void* GetThreadHandle(int index);
	bool IsThreadAlive(int index);
	unsigned int GetThreadID(int index);
	void SetInterval(int index, unsigned long interval);
	unsigned long GetInterval(int index);
};