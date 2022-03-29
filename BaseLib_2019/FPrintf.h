#pragma once
#include <stdio.h>
#include <Windows.h>
#include <io.h>
#include <direct.h>
#include <time.h>
#include "lib/Fmhha.h"
class __declspec(dllexport) FPrintf
{
public:
	FPrintf(char* filePath, char* fileName, unsigned int expiryDate = 60);
	virtual~FPrintf();

	enum
	{
		eBUFF = 1024,
	};
private:
	FILE* _write;
	Fmhha _lib;
	bool _isUse;
	unsigned int _expiryDate;

	int FindIndex(char* msg, char ch, int offset);
	void Erase(char* msg, unsigned int startIndex, unsigned int count);
	char* SubStr(char* msg, unsigned int startIndex, unsigned int count);
	void DeleteFolder() {}
	char* FormatV(char* msg, va_list args);
	void WriteFile(char* msg);

public:
	void AddLog(char* filePath, char* fileName);
	void IsUsePrintf(bool data = true);
	void PRINT_FA(char* msg, ...);
	void PRINT_F(char* msg);
};