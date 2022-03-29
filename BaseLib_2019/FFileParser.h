#pragma once
#include <stdio.h>
#include <Windows.h>
#include <io.h>
#include "lib\Fmhha.h"
class __declspec(dllexport) FFileParser
{
public:
	enum eMODE
	{
		INI = 0,
		XML = 1,
	};

	FFileParser(char* filePath, int mode = eMODE::INI);
	FFileParser(wchar_t* filePath, int mode = eMODE::INI);
	~FFileParser();

private:
	Fmhha _lib;

public:
	char*	GetString(char* cmd, char* defData);
	void	GetString(char* cmd, char* defData, char* returnValue);				// �Ǵ��� Ȯ�� ���غ�
	char*	GetStringW(wchar_t* cmd, wchar_t* defData);							// �Ǵ��� Ȯ�� ���غ�
	void	GetStringW(wchar_t* cmd, wchar_t* defData, wchar_t* returnValue);	// �Ǵ��� Ȯ�� ���غ�
	int		GetInt(char* cmd, int defData);
	void	GetInt(char* cmd, int defData, int& returnValue);					// �Ǵ��� Ȯ�� ���غ�
	int		GetIntW(wchar_t* cmd, int defData);									// �Ǵ��� Ȯ�� ���غ�
	void	GetIntW(wchar_t* cmd, int defData, int& returnValue);				// �Ǵ��� Ȯ�� ���غ�
	double	GetDouble(char* cmd, double defData);
	void	GetDouble(char* cmd, double defData, double& returnValue);			// �Ǵ��� Ȯ�� ���غ�
	double	GetDoubleW(wchar_t* cmd, double defData);							// �Ǵ��� Ȯ�� ���غ�
	void	GetDoubleW(wchar_t* cmd, double defData, double& returnValue);		// �Ǵ��� Ȯ�� ���غ�

	void	SetString(char* cmd, char* data);
	void	SetStringW(wchar_t* cmd, wchar_t* data);							// �Ǵ��� Ȯ�� ���غ�
	void	SetInt(char* cmd, int data);
	void	SetIntW(wchar_t* cmd, int data);									// �Ǵ��� Ȯ�� ���غ�
	void	SetIntStr(char* cmd, char* data);									// �Ǵ��� Ȯ�� ���غ�
	void	SetIntStrW(wchar_t* cmd, wchar_t* data);							// �Ǵ��� Ȯ�� ���غ�
	void	SetDouble(char* cmd, double data);
	void	SetDoubleW(wchar_t* cmd, double data);								// �Ǵ��� Ȯ�� ���غ�
	void	SetDoubleStr(char* cmd, char* data);								// �Ǵ��� Ȯ�� ���غ�
	void	SetDoubleStrW(wchar_t* cmd, wchar_t* data);							// �Ǵ��� Ȯ�� ���غ�
};

extern "C" __declspec(dllexport) FFileParser * CreateParser(char* filePath, int mode);
extern "C" __declspec(dllexport) const char* ParserGetString(FFileParser * file, char* cmd, char* defData);
extern "C" __declspec(dllexport) int ParserGetInt(FFileParser * file, char* cmd, int defData);
extern "C" __declspec(dllexport) double ParserGetDouble(FFileParser * file, char* cmd, double defData);
extern "C" __declspec(dllexport) void ParserSetString(FFileParser * file, char* cmd, char* value);
extern "C" __declspec(dllexport) void ParserSetInt(FFileParser * file, char* cmd, int value);
extern "C" __declspec(dllexport) void ParserSetDouble(FFileParser * file, char* cmd, double value);