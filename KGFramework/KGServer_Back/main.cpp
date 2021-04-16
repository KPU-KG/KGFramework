#include <iostream>
#include <WS2tcpip.h>
#include <windows.h>
#include "Data.h"
#include "Server.h"
#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")

Server S;

int main()
{
	S.Initialize();
}
 