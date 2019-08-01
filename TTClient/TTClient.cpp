/*
## 소켓 서버 : 1 v n - IOCP
1. socket()            : 소켓생성
2. connect()        : 연결요청
3. read()&write()
WIN recv()&send    : 데이터 읽고쓰기
4. close()
WIN closesocket    : 소켓종료
*/

#include "stdafx.h"

#include "TTNet/TTNet.h"

#include <iostream>
#include <fstream>
using namespace std;

#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER        1024
#define SERVER_IP        "10.110.31.180"
#define SERVER_PORT        3500

struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	WSABUF dataBuffer;
	int receiveBytes;
	int sendBytes;
};

int _tmain(int argc, _TCHAR* argv[])
{
	// Test Protocol
	int req_head_size = 24;
	int ack_head_size = 20;

	char buff[100];
	int pos = 0;

	//*
	// Req Packet Test
	flatbuffers::FlatBufferBuilder fbb_body;
	auto token = fbb_body.CreateString("howon");

	proto_battle::packet::LOGIN_ReqBuilder pbb_body(fbb_body);
	pbb_body.add_user_token(token);

	fbb_body.Finish(pbb_body.Finish());

	// Make Body ============
	cout << "fbb_body size : " << fbb_body.GetSize() << endl;
	fbb_body.GetBufferMinAlignment();
	cout << "min_body size : " << fbb_body.GetSize() << endl;

	memcpy_s(&buff[pos], fbb_body.GetSize(), fbb_body.GetBufferPointer(), fbb_body.GetSize());
	pos += fbb_body.GetSize();
	// ======================
	/*/
	// Ack Packet Test
	flatbuffers::FlatBufferBuilder fbb_head;

	proto_login::packet::head::AckBuilder pbb_head(fbb_head);
	pbb_head.add_protocol(proto_login::PROTOCOL::PING);
	pbb_head.add_result(proto_login::RESULT::SUCCESS);
	// pbb_head.add_result(proto_login::RESULT::RESULT_FAILURE);

	fbb_head.Finish(pbb_head.Finish());

	// Make Head ============
	cout << "fbb_head size : " << fbb_head.GetSize() << endl;
	fbb_head.GetBufferMinAlignment();
	cout << "min_head size : " << fbb_head.GetSize() << endl;

	memcpy_s(&buff[pos], fbb_head.GetSize(), fbb_head.GetBufferPointer(), fbb_head.GetSize());
	pos += fbb_head.GetSize();
	// ======================

	flatbuffers::FlatBufferBuilder fbb_body;

	proto_login::packet::body::LOGOUT_AckBuilder pbb_body(fbb_body);
	pbb_body.add_test1(1111);
	pbb_body.add_test2(2222);
	pbb_body.add_test3(3333);
	pbb_body.add_test4(4444);

	fbb_body.Finish(pbb_body.Finish());

	// Make Body ============
	cout << "fbb_body size : " << fbb_body.GetSize() << endl;
	fbb_body.GetBufferMinAlignment();
	cout << "min_body size : " << fbb_body.GetSize() << endl;

	memcpy_s(&buff[pos], fbb_body.GetSize(), fbb_body.GetBufferPointer(), fbb_body.GetSize());
	pos += fbb_body.GetSize();
	// ======================
	// */

	// ======================
	// Send pass
	cout << endl;
	cout << "Send packet size : " << pos << endl;
	cout << endl;
	// ======================


	// Recv =================
	//*
	// Req Packet Test
	auto req_body = flatbuffers::GetRoot<proto_battle::packet::LOGIN_Req>(buff);
	cout << "req_body->user_token : " << req_body->user_token() << endl;
	/*/
	// Ack Packet Test
	auto ack_head = flatbuffers::GetRoot<proto_login::packet::head::Ack>(&buff[0]);
	cout << "protocol : " << int(ack_head->protocol()) << endl;
	cout << "result : " << int(ack_head->result()) << endl;

	auto ack_body = flatbuffers::GetRoot<proto_login::packet::body::LOGOUT_Ack>(&buff[ack_head_size]);
	cout << "ack_body->test1 : " << ack_body->test1() << endl;
	cout << "ack_body->test2 : " << ack_body->test2() << endl;
	cout << "ack_body->test3 : " << ack_body->test3() << endl;
	cout << "ack_body->test4 : " << ack_body->test4() << endl;
	// */
	// ======================

	cout << endl;
	cout << endl;
	

	// Winsock Start - winsock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 0), &WSAData) != 0)
	{
		printf("Error - Can not load 'winsock.dll' file\n");
		return 1;
	}

	// 1. 소켓생성
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error - Invalid socket\n");
		return 1;
	}

	// 서버정보 객체설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);

	// 2. 연결요청
	if (connect(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		printf("Error - Fail to connect\n");
		// 4. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}
	else
	{
		printf("Server Connected\n* Enter Message\n->");
	}

	SOCKETINFO *socketInfo;
	DWORD sendBytes;
	DWORD receiveBytes;
	DWORD flags;

	while (1)
	{
		// 메시지 입력
		char messageBuffer[MAX_BUFFER];
		int i, bufferLen;
		for (i = 0; 1; i++)
		{
			messageBuffer[i] = getchar();
			if (messageBuffer[i] == '\n')
			{
				messageBuffer[i++] = '\0';
				break;
			}
		}
		bufferLen = i;

		socketInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
		memset((void*)socketInfo, 0x00, sizeof(struct SOCKETINFO));

		TTNet net;
		bool bPacket = true;

		if (0 == strcmp(messageBuffer, "ping")) {
			net.makePacketReq((uint16_t)proto_battle::PROTOCOL::PING);
		}
		else if (0 == strcmp(messageBuffer, "login")) {
			flatbuffers::FlatBufferBuilder fbb;
			auto token = fbb.CreateString("howon");

			proto_battle::packet::LOGIN_ReqBuilder pbb(fbb);
			pbb.add_user_token(token);

			fbb.Finish(pbb.Finish());

			net.makePacketReq((uint16_t)proto_battle::PROTOCOL::LOGIN, 1, fbb.GetBufferPointer(), fbb.GetSize());

			socketInfo->dataBuffer.len = net.getPacketLength();
			socketInfo->dataBuffer.buf = (char*)net.getPacketBuffer();
		}
		else if (0 == strcmp(messageBuffer, "logout")) {
			flatbuffers::FlatBufferBuilder fbb;

			proto_battle::packet::LOGOUT_ReqBuilder pbb(fbb);

			pbb.add_test1(1111);
			pbb.add_test2(2222);

			fbb.Finish(pbb.Finish());

			net.makePacketReq((uint16_t)proto_battle::PROTOCOL::LOGOUT, 2, fbb.GetBufferPointer(), fbb.GetSize());

			socketInfo->dataBuffer.len = net.getPacketLength();
			socketInfo->dataBuffer.buf = (char*)net.getPacketBuffer();
		}
		else if (0 == strcmp(messageBuffer, "data")) {
			flatbuffers::FlatBufferBuilder fbb;
			std::vector<uint8_t> byte_list;
			byte_list.push_back(11);
			byte_list.push_back(22);
			byte_list.push_back(33);
			byte_list.push_back(44);

			auto vec = fbb.CreateVector(byte_list);

			proto_battle::packet::DATA_ReqBuilder pbb(fbb);
			pbb.add_ubyte_list(vec);

			fbb.Finish(pbb.Finish());

			net.makePacketReq((uint16_t)proto_battle::PROTOCOL::DATA, 3, fbb.GetBufferPointer(), fbb.GetSize());
		}
		else {
			bPacket = false;
		}

		if (bPacket) {
			socketInfo->dataBuffer.len = net.getPacketLength();
			socketInfo->dataBuffer.buf = (char*)net.getPacketBuffer();
			sendBytes = socketInfo->dataBuffer.len;
		}
		else {
			socketInfo->dataBuffer.len = bufferLen;
			socketInfo->dataBuffer.buf = messageBuffer;
		}

		// 3-1. 데이터 쓰기
		int sendBytes = send(listenSocket, socketInfo->dataBuffer.buf, socketInfo->dataBuffer.len, 0);
		if (sendBytes > 0)
		{
			printf("TRACE - Send message : %s (%d bytes)\n", messageBuffer, sendBytes);
			// 3-2. 데이터 읽기
			int receiveBytes = recv(listenSocket, messageBuffer, MAX_BUFFER, 0);
			if (receiveBytes > 0)
			{
				printf("TRACE - Recv message : %s (%d bytes)\n* Enter Message\n->", messageBuffer, receiveBytes);
			}
		}

	}

	// 4. 소켓종료
	closesocket(listenSocket);

	// Winsock End
	WSACleanup();

	return 0;
}