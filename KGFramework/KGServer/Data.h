#pragma once
#include <WS2tcpip.h>
#include "MathHelper.h"

#define MAX_BUFFER        1024
#define SERVER_PORT       9000
#define Recv_Mode 0
#define Send_Mode 1
#define MAX_Client 4

struct OVERLAPPED_EX {
	WSAOVERLAPPED over;
	WSABUF	wsabuf;
	char	net_buf[MAX_BUFFER];
	bool	is_recv;
};

struct SOCKETINFO
{
	OVERLAPPED_EX	recv_over;
	SOCKET	socket;
	int		id;
};

typedef struct // 소켓 정보
{
	SOCKET client_handle;
	SOCKADDR_IN client_addr;
	int client_id = -1;
} PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

typedef struct // 소켓 버퍼
{
	OVERLAPPED overlapped;
	char buffer[MAX_BUFFER];
	bool incoming_data;
	WSABUF wsaBuf;
} PER_IO_DATA, * LPPER_IO_DATA;

struct InputData {
	bool isWClicked = false;
	bool isAClicked = false;
	bool isSClicked = false;
	bool isDClicked = false;
	bool is1Clicked = false;
	bool is2Clicked = false;
	bool is3Clicked = false;
};

struct Input_Datas {
	InputData data[MAX_Client];
};

struct PlayerData {
	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT4 rotation = DirectX::XMFLOAT4(0, 0, 0, 1); 
	char currentAnimation = ' ';
};

struct Player_Datas {
	PlayerData data[MAX_Client];
};

struct User_Datas {
	Player_Datas playerdata;
	Input_Datas input;
};


// 패킷 구조 

/*
플레이어
위치 정보 - 트랜스폼 따로
상태 - 애니메이션, ?

체력 판정계산 등은 서버에서 이벤트 처리 후 상태만 전송

오브젝트

적 - 젠되는 구간 정해져 있음
위치 정보
상태

탄 - 발사만 이벤트 처리 후 서버에서 처리?
-> 패킷에 포함할 경우
위치 정보
탄 정보(공격력 등)

---------------------------

스타트 씬 - 서버 연결, id? 부여, 연결하고 저장 패킷은 시작 정보 전송 등
->
게임 씬 - 실제 게임 처리 및 패킷 전송(업데이트 + 플레이어 정보 + )
적 정보, 탄 정보(서버에서 처리할 경우 발사 이벤트로만 처리)
물리처리
->
게임 종료 - 서버 연결 해제
->
스타트 씬



맵 작업 - 소품은 많이 찾아서 배치했음 
건물은 찾기가 어려워서 유료 몇개 찾아놓은게 대다수라 배치 못함
건물 모델에 pbr텍스처 붙여보려함

패킷 구성 - 주기적으로 보내는 패킷, 이벤트 패킷으로 분리해서 
주기적으로 보내는 패킷 - 위치, 상태, id
이벤트 패킷 - 이벤트 종류(몹 젠같은건 위치도 포함시켜서 종류 나누기 -  ex)1구역 몹젠)

---------------------

-------------------------
*/