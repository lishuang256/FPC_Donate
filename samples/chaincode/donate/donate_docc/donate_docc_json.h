#pragma once

#include "shim.h"

#include <string>
#include <vector>
#include <cstdint>

struct Donate_tx {
	std::string hres;
	int dep;
	std::string eres;
	std::string comx;
	std::string comy;
	std::string dt;
};

struct Deliver1_tx {
	std::string hres;
	int dep;
    std::string dt;
};

struct Store_tx {
	std::string hres;
	std::string dt;
};

struct Ship_tx {
	std::string hres;
	int dep;
	std::string dt;
};

struct Dis_tx {
	std::string hres;
	std::string comqx;
	std::string comqy;
	std::string dt;
};

struct Deliver2_tx {
	std::string hres;
	int dep;
	std::string dt;
};

struct Receive_tx {
	std::string hres;
	std::string hdf;
	std::string edf;
	std::string dt;
};

struct Refund_tx {
	std::string hres;
	int tp1;
	int co;
	int tp2;
	std::string dt;
};

enum State {
	donate,
	deliver1,
	store,
	ship,
	deliver2,
	receive
};

struct don {
	State state;
	int deposit;
	std::string DepositList;
};

//std::string bytesToBinaryString(const std::vector<uint8_t>& bytes);
//std::vector<uint8_t> binaryStringToBytes(const std::string& binaryStr);
std::string marshal_Donate(Donate_tx* donate);
std::string marshal_Deliver1(Deliver1_tx* deliver1);
std::string marshal_Store(Store_tx* store);
std::string marshal_Ship(Ship_tx* ship);
std::string marshal_Dis(Dis_tx* dis);
std::string marshal_Deliver2(Deliver2_tx* deliver2);
std::string marshal_Receive(Receive_tx* receive);
std::string marshal_Refund(Refund_tx* refund);
std::string marshal_don(don* Don);
int unmarshal_Donate(Donate_tx* donate, const char* json_bytes, uint32_t json_len);
int unmarshal_don(don* Don, const char* json_bytes, uint32_t json_len);
