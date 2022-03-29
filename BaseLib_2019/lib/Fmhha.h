#pragma once
#include <Windows.h>
class __declspec(dllexport) Fmhha
{
public:
	Fmhha();
	virtual ~Fmhha();

private:
	unsigned long long _startTime;

public:
	bool LibraryPermit();

private:
	unsigned long long GetElapsed()
	{
		return GetTickCount64() - _startTime;
	}
};