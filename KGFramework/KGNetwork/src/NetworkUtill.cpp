#include "pch.h"
#include "NetworkUtill.h"
#include <Windows.h>
#include <iostream>
void display_error(const wchar_t* msg, int err_no)
{
	WCHAR* lpMsgBuffer;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuffer, 0, NULL);
	MessageBox(NULL, lpMsgBuffer, msg, MB_OK);
	LocalFree(lpMsgBuffer);
	exit(-1);
}