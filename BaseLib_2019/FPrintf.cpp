#include "FPrintf.h"

static char _printfPath[256]{ 0, };
static char _printfName[256]{ 0, };
static char _subBuff[1024]{ 0, };
static char _formatRet[2049]{ 0, };

FPrintf::FPrintf(char* filePath, char* fileName, unsigned int expiryDate)
{
	_lib = Fmhha();
	sprintf_s(_printfPath, 256, "%s", filePath);
	sprintf_s(_printfName, 256, "%s", fileName);
	_isUse = true;
	_expiryDate = expiryDate;
}

FPrintf::~FPrintf()
{
}

int FPrintf::FindIndex(char* msg, char ch, int offset)
{
	if (_lib.LibraryPermit() == false)
	{
		return -1;
	}

	char* temp = msg;
	*(temp = temp + offset);
	while (1)
	{
		if (*(temp) == 0)
			return -1;
		if (*(temp++) == ch)
			break;
	}

	return (int)(temp - msg) - 1;
}

void FPrintf::Erase(char* msg, unsigned int startIndex, unsigned int count)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	char* temp = msg;
	if (strlen(temp) <= startIndex)
		return;

	if (strlen(temp) <= startIndex + count)
		count = strlen(temp) - startIndex;
	
	char* left = new char[startIndex + 1]{ 0, };
	for (unsigned int i = 0; i < startIndex; i++)
	{
		left[i] = *(temp++);
	}
	for (unsigned int i = 0; i < count; i++)
	{
		*(temp++);
	}
	unsigned int rightLength = strlen(temp);
	char* right = new char[rightLength + 1]{ 0, };
	for (unsigned int i = 0; i < rightLength; i++)
	{
		right[i] = *(temp++);
	}
	sprintf_s(msg, strlen(msg), "%s%s", left, right);
	delete[] left;
	delete[] right;
}

char* FPrintf::SubStr(char* msg, unsigned int startIndex, unsigned int count)
{
	if (_lib.LibraryPermit() == false)
	{
		return msg;
	}

	ZeroMemory(_subBuff, sizeof(_subBuff));
	char* temp = msg;
	for (unsigned int i = 0; i < startIndex; i++)
	{
		if (*(temp) == 0)
			return temp;
		*(temp++);
	}
	if (*(temp) == 0)
		return temp;
	for (unsigned int i = 0; i < count; i++)
	{
		_subBuff[i] = *(temp++);
		if (*(temp) == 0)
			return _subBuff;
	}

	return _subBuff;
}

char* FPrintf::FormatV(char* msg, va_list args)
{
	if (_lib.LibraryPermit() == false)
	{
		return msg;
	}

	if (msg == NULL || *msg == 0)
		return msg;

	bool repeat = true;
	int pp = 0;

	ZeroMemory(_formatRet, sizeof(_formatRet));
	char* point = new char[20]{ 0, };
	char* temp = new char[2049];
	sprintf_s(temp, 2049, "%s", msg);
	int pos = FindIndex(temp, '%', 0);
	while (pos != -1)
	{
		if (repeat == true)
		{
			sprintf_s(_formatRet, eBUFF + eBUFF + 1, "%s%s", _formatRet, SubStr(temp, 0, pos));
			repeat = false;
		}
		char buff[1024] = { 0, };
		char chPos = *(temp + pos + pp);
		if (chPos == 'd')
		{
			sprintf_s(point, 20, "%s%c", point, chPos);
			sprintf_s(buff, 1024, point, va_arg(args, int));
		}
		else if (chPos == 'f')
		{
			sprintf_s(point, 20, "%s%c", point, chPos);
			sprintf_s(buff, 1024, point, va_arg(args, double));
		}
		else if (chPos == 'g')
		{
			sprintf_s(point, 20, "%s%c", point, chPos);
			sprintf_s(buff, 1024, point, va_arg(args, double));
		}
		else if (chPos == 's')
		{
			sprintf_s(point, 20, "%s%c", point, chPos);
			sprintf_s(buff, 1024, point, va_arg(args, char*));
		}
		else if (chPos == 'c')
		{
			sprintf_s(point, 20, "%s%c", point, chPos);
			sprintf_s(buff, 1024, "%c", va_arg(args, char));
		}
		else if (chPos == 'x')
		{
			sprintf_s(point, 20, "%s%c", point, chPos);
			sprintf_s(buff, 1024, point, va_arg(args, char*));
		}
		else
		{
			sprintf_s(point, 20, "%s%c", point, chPos);
			pp++;
			continue;
		}
		sprintf_s(_formatRet, eBUFF + eBUFF + 1, "%s%s", _formatRet, buff);
		memset(point, 0, 20);
		Erase(temp, 0, pos + pp + 1);
		pos = FindIndex(temp, '%', 0);
		repeat = true;
		pp = 0;
	}


	delete[] point;
	delete[] temp;
	return _formatRet;
}

void FPrintf::WriteFile(char* msg)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	time_t curTime = time(NULL);
	tm* _local = new tm();
	localtime_s(_local, &curTime);
	char timeBuf[2048] = { 0, };
	sprintf_s(timeBuf, 2048, "[%04d-%02d-%02d %02d:%02d:%02d]	",
		_local->tm_year + 1900, _local->tm_mon + 1, _local->tm_mday, _local->tm_hour, _local->tm_min, _local->tm_sec);

	char filePath[256] = { 0, };
	sprintf_s(filePath, 256, "%s\\%04d%02d%02d", _printfPath, _local->tm_year + 1900, _local->tm_mon + 1, _local->tm_mday);
	char dirName[256];
	int ret = _access_s(filePath, 0);
	if (ret != 0)
	{
		char* underPath = filePath;
		char* thisPath = dirName;
		while (*underPath)
		{
			if (('\\' == *underPath) || ('/' == *underPath))
			{
				if (':' != *(underPath - 1))
				{
					ret = _access_s(dirName, 0);
					if (ret != 0)
					{
						_mkdir(dirName);
					}
				}
			}
			*thisPath++ = *underPath++;
			*thisPath = '\0';
		}
		_mkdir(filePath);
	}

	char buff[2048] = { 0, };
	sprintf_s(buff, 2048, "%s\\%s.log", filePath, _printfName);
	ret = _access_s(buff, 0);

	if (ret != 0)
	{
		fopen_s(&_write, buff, "w");
		fputs(timeBuf, _write);
		fputs(msg, _write);
		fputs("\n", _write);
		fclose(_write);
	}
	else
	{
		fopen_s(&_write, buff, "a");
		fputs(timeBuf, _write);
		fputs(msg, _write);
		fputs("\n", _write);
		fclose(_write);
	}

	delete _local;
}

void FPrintf::AddLog(char* filePath, char* fileName)
{
	sprintf_s(_printfPath, 256, "%s", filePath);
	sprintf_s(_printfName, 256, "%s", fileName);
}
void FPrintf::IsUsePrintf(bool data)
{
	_isUse = data;
}

void FPrintf::PRINT_FA(char* msg, ...)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	va_list argList;
	va_start(argList, msg);
	char* temp = FormatV(msg, argList);
	va_end(argList);
	char saveData[2049] = { 0, };
	memcpy(saveData, temp, strlen(temp));
	WriteFile(saveData);
}

void FPrintf::PRINT_F(char* msg)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}

	char saveData[2049] = { 0, };
	memcpy(saveData, msg, strlen(msg));
	WriteFile(saveData);
}