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

typedef struct // ���� ����
{
	SOCKET client_handle;
	SOCKADDR_IN client_addr;
	int client_id = -1;
} PER_HANDLE_DATA, * LPPER_HANDLE_DATA;

typedef struct // ���� ����
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


// ��Ŷ ���� 

/*
�÷��̾�
��ġ ���� - Ʈ������ ����
���� - �ִϸ��̼�, ?

ü�� ������� ���� �������� �̺�Ʈ ó�� �� ���¸� ����

������Ʈ

�� - ���Ǵ� ���� ������ ����
��ġ ����
����

ź - �߻縸 �̺�Ʈ ó�� �� �������� ó��?
-> ��Ŷ�� ������ ���
��ġ ����
ź ����(���ݷ� ��)

---------------------------

��ŸƮ �� - ���� ����, id? �ο�, �����ϰ� ���� ��Ŷ�� ���� ���� ���� ��
->
���� �� - ���� ���� ó�� �� ��Ŷ ����(������Ʈ + �÷��̾� ���� + )
�� ����, ź ����(�������� ó���� ��� �߻� �̺�Ʈ�θ� ó��)
����ó��
->
���� ���� - ���� ���� ����
->
��ŸƮ ��



�� �۾� - ��ǰ�� ���� ã�Ƽ� ��ġ���� 
�ǹ��� ã�Ⱑ ������� ���� � ã�Ƴ����� ��ټ��� ��ġ ����
�ǹ� �𵨿� pbr�ؽ�ó �ٿ�������

��Ŷ ���� - �ֱ������� ������ ��Ŷ, �̺�Ʈ ��Ŷ���� �и��ؼ� 
�ֱ������� ������ ��Ŷ - ��ġ, ����, id
�̺�Ʈ ��Ŷ - �̺�Ʈ ����(�� �������� ��ġ�� ���Խ��Ѽ� ���� ������ -  ex)1���� ����)

---------------------

-------------------------
*/