#include "pch.h"
#include "ServerUtill.h"
#include <Windows.h>
#include <iostream>
void display_error(const char* msg, int err_no)
{
	WCHAR* lpMsgBuffer;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuffer, 0, NULL);
	std::cout << msg;
	std::wcout << lpMsgBuffer << std::endl;
	LocalFree(lpMsgBuffer);
}