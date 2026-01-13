#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct RequestStruct {
    std::string CO;
    std::string amount0;  		// donation amount in need
};

struct ResponseStruct {
    std::string amount;
    std::string ml;				// material list
    std::string comx;
    std::string comy;
    std::string comqx;
    std::string comqy;
    std::string r;				// first random number
    std::string rj;				// second random number
    //RequestStruct req;
    std::string dt;		// date time
};

struct FeedbackStruct {
    std::string mlj;
    std::string eres;
    std::string dt;		// date time
};

struct BigIntegerStruct {
    std::string num;
};

std::string marshal_Res(ResponseStruct* respoonse);
int unmarshal_Res(ResponseStruct* respoonse, const char* json_bytes, uint32_t json_len);
std::string marshal_Int(BigIntegerStruct* Int);
int unmarshal_Int(BigIntegerStruct* Int, const char* json_bytes, uint32_t json_len);
