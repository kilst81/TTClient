#pragma once

#include "../flatbuffers/flatbuffers.h"
#include "../protocol/btc_login_generated.h"
#include "../protocol/btc_battle_generated.h"

typedef unsigned char byte;



struct TT_PacketHead
{
	// data
	uint16_t size		= 0;
	uint16_t protocol	= 0;
	uint32_t value		= 0;

	// function
	// Req
	void setSession(uint32_t session_) { value = session_; }
	uint32_t getSession() const { return value; }

	// Ack
	void setResultCode(uint16_t result_, uint16_t code_) { value = result_ << 8; value += code_; }
	uint16_t getResult() const { return value >> 8; }
	uint16_t getCode() const { return value & 0x0000FFFF; }
};


class TTNet {

public :
	static const int PACKET_SIZE_BUFFER = 8192;
	static const int PACKET_SIZE_HEADER = sizeof(TT_PacketHead);


public :
	byte buf[PACKET_SIZE_BUFFER] = {0};


public :
	uint16_t getPacketLength() const { return *(uint16_t*)buf; }
	byte* getPacketBuffer() const { return (byte*)buf; }


public :
	TT_PacketHead* getHead() const { return (TT_PacketHead*)buf; }
	byte* getBody() const { return (byte*)(&buf[PACKET_SIZE_HEADER]); }


public :
	void makePacketReq(uint16_t protocol_, uint32_t session_ = 0, const byte* buf_ = nullptr, int len_ = 0)
	{
		TT_PacketHead head;
		head.size = PACKET_SIZE_HEADER + len_;
		head.protocol = protocol_;
		head.value = session_;

		memcpy_s(buf, PACKET_SIZE_HEADER, &head, PACKET_SIZE_HEADER);
		memcpy_s(&buf[PACKET_SIZE_HEADER], len_, buf_, len_);
	}

	void makePacketAck(uint16_t protocol_, const byte* buf_ = nullptr, int len_ = 0)
	{
		TT_PacketHead head;
		head.size = PACKET_SIZE_HEADER + len_;
		head.protocol = protocol_;

		memcpy_s(buf, PACKET_SIZE_HEADER, &head, PACKET_SIZE_HEADER);
		if (buf_)
		{
			memcpy_s(&buf[PACKET_SIZE_HEADER], len_, buf_, len_);
		}
	}

	void makePacketResult(uint16_t protocol_, uint16_t result_, uint16_t code_ = 0)
	{
		TT_PacketHead head;
		head.size = PACKET_SIZE_HEADER;
		head.protocol = protocol_;
		head.setResultCode(result_, code_);

		memcpy_s(buf, PACKET_SIZE_HEADER, &head, PACKET_SIZE_HEADER);
	}

};


