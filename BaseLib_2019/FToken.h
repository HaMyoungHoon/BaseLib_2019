#pragma once
#include <Windows.h>
#include "lib/Fmhha.h"

class __declspec(dllexport) FToken
{
public:
	FToken();
	virtual ~FToken();

private:
	enum
	{
		eMAX_TOKEN = 100,
	};

	TCHAR	_sepTChar[50];
	BYTE	_sepChar[50];
	LPTSTR	_tokenTChar[eMAX_TOKEN];
	PBYTE	_tokenChar[eMAX_TOKEN];
	INT		_count;
	Fmhha	_lib;
	VOID	ReleaseAllToken();

	INT		FindString(LPCTSTR source, LPCTSTR sep);
	VOID	SetSeperator(LPCTSTR sep);

	INT		FindByte(PBYTE source, PBYTE sep);
	INT		FindByteSize(LONG size, PBYTE source, PBYTE sep);
	VOID	SetSeperatorByte(PBYTE sep);

public:
	INT			GetCount(VOID) { return _count; }

	VOID		Split(LPCTSTR source, LPCTSTR sep);
	LPTSTR		GetToken(INT index);
	VOID		SplitByte(PBYTE source, PBYTE sep);
	PBYTE		GetTokenByte(INT index);
};