#include "FToken.h"

FToken::FToken()
{
	_lib = Fmhha();
	_count = 0;

	for (LONG i = 0; i < eMAX_TOKEN; i++)
	{
		_tokenTChar[i] = NULL;
		_tokenChar[i] = NULL;
	}
}

FToken::~FToken()
{
	ReleaseAllToken();
}

VOID FToken::ReleaseAllToken()
{
	for (LONG i = 0; i < _count; i++)
	{
		if (_tokenTChar[i])
		{
			delete[] _tokenTChar[i];
			_tokenTChar[i] = NULL;

		}
		if (_tokenChar[i])
		{
			delete[] _tokenChar[i];
			_tokenChar[i] = NULL;
		}
	}
}

INT FToken::FindString(LPCTSTR source, LPCTSTR sep)
{
	if (_lib.LibraryPermit() == false)
	{
		return -1;
	}

	LPTSTR	temp = (LPTSTR)source;
	INT		sepSize = lstrlen(sep);

	while (memcmp(temp, sep, sepSize * sizeof(*source)) != 0)
	{
		if (*(temp++) == 0)
			return -1;
	}

	return (INT)(temp - (LPTSTR)source);
}

VOID FToken::Split(LPCTSTR source, LPCTSTR sep)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	SetSeperator(sep);

	ReleaseAllToken();

	LPTSTR	temp = (LPTSTR)source;
	INT		sepSize = lstrlen(_sepTChar);
	INT		pos = 0;
	INT		token = 0;

	while (true)
	{
		INT nR = FindString((LPCTSTR)(temp + pos), (LPCTSTR)_sepTChar);
		if (nR != -1)
		{
			INT nLenToken = nR;
			INT nSize = nLenToken + 1;
			_tokenTChar[token] = new TCHAR[nSize];
			memcpy(_tokenTChar[token], temp + pos, nLenToken * sizeof(*source));
			memset((PVOID)&_tokenTChar[token++][nLenToken], 0x00, sizeof(*source));
			pos += (sepSize + nR);
		}
		else
		{
			INT nLenToken = lstrlen(temp + pos);
			INT nSize = nLenToken + 1;
			_tokenTChar[token] = new TCHAR[nSize];
			memcpy(_tokenTChar[token], temp + pos, nLenToken * sizeof(*source));
			memset((PVOID)&_tokenTChar[token++][nLenToken], 0x00, sizeof(*source));
			break;
		}
	}
	_count = token;
}

LPTSTR FToken::GetToken(INT index)
{
	static PTCHAR empty = L"";
	if (_lib.LibraryPermit() == false)
	{
		return empty;
	}

	if (index >= _count)
		return empty;

	return _tokenTChar[index];
}

VOID FToken::SetSeperator(LPCTSTR sep)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	lstrcpy(_sepTChar, sep);
}

INT FToken::FindByte(PBYTE source, PBYTE sep)
{
	if (_lib.LibraryPermit() == false)
	{
		return -1;
	}
	PBYTE addr = source;

	LONG sepSize = strlen((PCHAR)sep);

	while (memcmp(addr, sep, sepSize) != 0)
	{
		if (*(addr++) == 0x0)
			return -1;
	}

	return (INT)(addr - source);
}

INT FToken::FindByteSize(LONG size, PBYTE source, PBYTE sep)
{
	if (_lib.LibraryPermit() == false)
	{
		return -1;
	}

	PBYTE addr = source;

	LONG sepSize = strlen((PCHAR)sep);

	for (LONG i = 0; i < size; i++)
	{
		if (memcmp(addr, sep, sepSize) == 0)
		{
			return (INT)(addr - source);
		}
		addr++;
	}
	return -1;
}

VOID FToken::SplitByte(PBYTE source, PBYTE sep)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	SetSeperatorByte(sep);

	ReleaseAllToken();

	PBYTE	temp = source;
	INT		sepSize = strlen((PCHAR)_sepChar);
	INT		pos = 0;
	INT		token = 0;

	while (true)
	{
		INT nR = FindByte((temp + pos), _sepChar);
		if (nR != -1)
		{
			INT nLenToken = nR;
			INT nSize = nLenToken + 1;
			_tokenChar[token] = new BYTE[nSize];
			memcpy(_tokenChar[token], temp + pos, nLenToken * sizeof(*source));
			memset((PVOID)&_tokenChar[token++][nLenToken], 0x00, sizeof(*source));
			pos += (sepSize + nR);
		}
		else
		{
			INT nLenToken = strlen((char*)temp + pos);
			INT nSize = nLenToken + 1;
			_tokenChar[token] = new BYTE[nSize];
			memcpy(_tokenChar[token], temp + pos, nLenToken * sizeof(*source));
			memset((PVOID)&_tokenChar[token++][nLenToken], 0x00, sizeof(*source));
			break;
		}
	}
	_count = token;
}

PBYTE FToken::GetTokenByte(INT index)
{
	static PCHAR szEmpty = "";
	if (_lib.LibraryPermit() == false)
	{
		return (PBYTE)szEmpty;
	}

	if (index >= _count)
		return (PBYTE)szEmpty;

	return _tokenChar[index];
}

VOID FToken::SetSeperatorByte(PBYTE sep)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	strcpy_s((PCHAR)_sepChar, 50, (CONST PCHAR)sep);
}