#include "shim.h"
#include "logging.h"
#include "BigInteger.h"
#include "donate_dicc.h"
#include "donate_dicc_json.h"

#include <tuple>
#include <openssl/evp.h>

#define OK "OK"
#define MAX_MSP_ID_LEN 128
#define MAX_DN_LEN 256
#define MAX_VALUE_SIZE 1024
#define RANDOM_SIZE 16
//#define RANDOM_SIZE 4

#define DONATE_NO_DISTRIBUTES "DONATE_NO_DISTRIBUTES"
#define DATA_NOT_EXISTING "DATA_NOT_EXISTING"
#define TRUE "TRUE"
#define FALSE "FALSE"

const std::string SEP = ".";
const std::string PREFIX = SEP + "somePrefix" + SEP;

//Parameters of Secp256k1
const BigInteger GX("0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798");
const BigInteger GY("0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8");
const BigInteger AA("0");
const BigInteger BB("7");
const BigInteger PP("0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F");

//BigInteger HX("0");
//BigInteger HY("0");
BigInteger HX("0xC6047F9441ED7D6D3045406E95C07CD85C778E4B8CEF3CA7ABAC09B95C709EE5");
BigInteger HY("0x1AE168FEA63DC339A3C58419466EFAEEF7F632653266D0E1236431A950CFE52A");

const BigInteger zero("0");
const BigInteger one("1");
const BigInteger two("2");
const BigInteger three("3");

std::string to_hex_string(const unsigned char* data, unsigned int len) {
    const char hex_chars[] = "0123456789ABCDEF";
    std::string result;
    result.reserve(len * 2);
    for (unsigned int i = 0; i < len; ++i) {
        result += hex_chars[(data[i] >> 4) & 0x0F];
        result += hex_chars[data[i] & 0x0F];
    }
    return result;
}

std::string sha256(const std::string& msg) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
	EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, msg.c_str(), msg.size());
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);
    return "0x" + to_hex_string(hash, hash_len);
}

std::string getRand() {
    uint8_t buffer[RANDOM_SIZE];
    get_random_bytes(buffer, RANDOM_SIZE);
        // Limit random bytes to printable characters ('0' to '9')
    for (size_t i = 0; i < RANDOM_SIZE; ++i) {
       buffer[i] = '0' + (buffer[i] % 10);
    }
    std::string random_buffer(buffer, buffer + RANDOM_SIZE);
    return random_buffer;
}

BigInteger addmod(const BigInteger& a, const BigInteger& b, const BigInteger& mod) {
    return (a + b) % mod;
}

BigInteger mulmod(const BigInteger& a, const BigInteger& b, const BigInteger& mod) {
    return (a * b) % mod;
}

std::tuple<BigInteger, BigInteger> _jAdd(const BigInteger& x1, const BigInteger& z1, const BigInteger& x2, const BigInteger& z2) {
    BigInteger x3 = addmod(mulmod(z2, x1, PP), mulmod(x2, z1, PP), PP);
    BigInteger z3 = mulmod(z1, z2, PP);
    return std::make_tuple(x3, z3);
}

std::tuple<BigInteger, BigInteger> _jSub(const BigInteger& x1, const BigInteger& z1, const BigInteger& x2, const BigInteger& z2) {
    BigInteger x3 = addmod(mulmod(z2, x1, PP), mulmod(PP - x2, z1, PP), PP);
    BigInteger z3 = mulmod(z1, z2, PP);
    return std::make_tuple(x3, z3);
}

std::tuple<BigInteger, BigInteger> _jMul(const BigInteger& x1, const BigInteger& z1, const BigInteger& x2, const BigInteger& z2) {
    BigInteger x3 = mulmod(x1, x2, PP);
    BigInteger z3 = mulmod(z1, z2, PP);
    return std::make_tuple(x3, z3);
}

std::tuple<BigInteger, BigInteger> _jDiv(const BigInteger& x1, const BigInteger& z1, const BigInteger& x2, const BigInteger& z2) {
    BigInteger x3 = mulmod(x1, z2, PP);
    BigInteger z3 = mulmod(z1, x2, PP);
    return std::make_tuple(x3, z3);
}

BigInteger _inverse(const BigInteger& val) {
    BigInteger t("0");
    BigInteger newT("1");
    BigInteger r = PP;
    BigInteger newR = val;
    BigInteger q;

    while (newR != zero) {
        q = r / newR;
        t = newT;
		newT = addmod(t, PP - mulmod(q, newT, PP), PP);
		r = newR;
        newR = r - q * newR;
    }
    return t;
}

std::tuple<BigInteger, BigInteger, BigInteger> _ecAdd(const BigInteger& x1, const BigInteger& y1, const BigInteger& z1, const BigInteger& x2, const BigInteger& y2, const BigInteger& z2) {
    if (x1 == zero && y1 == zero) return std::make_tuple(x2, y2, z2);
    if (x2 == zero && y2 == zero) return std::make_tuple(x1, y1, z1);

    BigInteger lx, lz, da, db;
    if (x1 == x2 && y1 == y2) {
        std::tie(lx, lz) = _jMul(x1, z1, x1, z1);
        std::tie(lx, lz) = _jMul(lx, lz, three, one);
        std::tie(lx, lz) = _jAdd(lx, lz, AA, one);
        std::tie(da, db) = _jMul(y1, z1, two, one);
    } else {
        std::tie(lx, lz) = _jSub(y2, z2, y1, z1);
        std::tie(da, db) = _jSub(x2, z2, x1, z1);
    }

    std::tie(lx, lz) = _jDiv(lx, lz, da, db);

    BigInteger x3, y3, z3;
    std::tie(x3, da) = _jMul(lx, lz, lx, lz);
    std::tie(x3, da) = _jSub(x3, da, x1, z1);
    std::tie(x3, da) = _jSub(x3, da, x2, z2);

    std::tie(y3, db) = _jSub(x1, z1, x3, da);
    std::tie(y3, db) = _jMul(y3, db, lx, lz);
    std::tie(y3, db) = _jSub(y3, db, y1, z1);

    if (!(da == db)) {
        x3 = mulmod(x3, db, PP);
        y3 = mulmod(y3, da, PP);
        z3 = mulmod(da, db, PP);
    } else {
        z3 = da;
    }

    return std::make_tuple(x3, y3, z3);
}

std::tuple<BigInteger, BigInteger, BigInteger> _ecDouble(const BigInteger& x1, const BigInteger& y1, const BigInteger& z1) {
    return _ecAdd(x1, y1, z1, x1, y1, z1);
}

std::tuple<BigInteger, BigInteger, BigInteger> _ecMul(const BigInteger& d, const BigInteger& x1, const BigInteger& y1, const BigInteger& z1) {
    BigInteger remaining = d;
    BigInteger px = x1, py = y1, pz = z1;
    BigInteger acx = zero, acy = zero, acz = one;

    if (d == zero) return std::make_tuple(zero, zero, one);

    while (remaining != zero) {
        if ((remaining & one) != zero) {
            std::tie(acx, acy, acz) = _ecAdd(acx, acy, acz, px, py, pz);
        }
        remaining = remaining / two;
        std::tie(px, py, pz) = _ecDouble(px, py, pz);
    }
    return std::make_tuple(acx, acy, acz);
}

std::tuple<BigInteger, BigInteger> ecadd(const BigInteger& x1, const BigInteger& y1, const BigInteger& x2, const BigInteger& y2) {
    BigInteger x3, y3, z;
    std::tie(x3, y3, z) = _ecAdd(x1, y1, one, x2, y2, one);
    z = _inverse(z);
    x3 = mulmod(x3, z, PP);
    y3 = mulmod(y3, z, PP);
    return std::make_tuple(x3, y3);
}

std::tuple<BigInteger, BigInteger> ecmul(const BigInteger& x1, const BigInteger& y1, const BigInteger& scalar) {
    BigInteger x2, y2, z;
    std::tie(x2, y2, z) = _ecMul(scalar, x1, y1, one);
    z = _inverse(z);
    x2 = mulmod(x2, z, PP);
    y2 = mulmod(y2, z, PP);
    return std::make_tuple(x2, y2);
}

std::tuple<BigInteger, BigInteger> genProof(const BigInteger& r1, const BigInteger& r2) {
    BigInteger cm1x, cm1y, cm2x, cm2y, comx, comy;
    std::tie(cm1x, cm1y) = ecmul(GX, GY, r1);	// g^r1
    std::tie(cm2x, cm2y) = ecmul(HX, HY, r2);	// h^r2
    std::tie(comx, comy) = ecadd(cm1x, cm1y, cm2x, cm2y);	// com = g^r1 * h^r2
    return std::make_tuple(comx, comy);
}

std::string Distribute(std::string amount, std::string ml, std::string comx, std::string comy, std::string r, std::string rj, std::string dt, shim_ctx_ptr_t ctx) {
    std::string Sum_name = "Sum";		// Sum of amount
    uint32_t sum_bytes_len = 0;
    uint8_t sum_bytes[MAX_VALUE_SIZE];
    get_state(Sum_name.c_str(), sum_bytes, sizeof(sum_bytes), &sum_bytes_len, ctx);
    
    std::string X_name = "X";			// Sum of r
    uint32_t x_bytes_len = 0;
    uint8_t x_bytes[MAX_VALUE_SIZE];
    get_state(X_name.c_str(), x_bytes, sizeof(x_bytes), &x_bytes_len, ctx);
    
    std::string XJ_name = "XJ";			// Sum of rj
    uint32_t xj_bytes_len = 0;
    uint8_t xj_bytes[MAX_VALUE_SIZE];
    get_state(XJ_name.c_str(), xj_bytes, sizeof(xj_bytes), &xj_bytes_len, ctx);
    
    BigInteger sum, x, xj;
    if (sum_bytes_len == 0 && x_bytes_len == 0 && xj_bytes_len == 0) {
        sum = zero;
        x = zero;
        xj = zero;
    } 
    else {
        BigIntegerStruct a, b, c;
        unmarshal_Int(&a, (const char*)sum_bytes, sum_bytes_len);
        unmarshal_Int(&b, (const char*)x_bytes, x_bytes_len);
        unmarshal_Int(&c, (const char*)xj_bytes, xj_bytes_len);

        BigInteger d(a.num);
        BigInteger e(b.num);
        BigInteger f(c.num);
        sum = d;
        x = e;
        xj = f;
    }

    BigInteger comqx, comqy;
    BigInteger num(amount);
    BigInteger r1(r);
    BigInteger r2(rj);

    sum = sum + num;					// Update sum, x, xj
    x = x + r1;
    xj = xj + r2;
    std::tie(comqx, comqy) = genProof(num, r2);
    
    std::string hres = sha256(amount + ml + comx + comy + r + rj + dt);
    std::string Response_name(PREFIX + "Res" + SEP + hres + SEP);
	ResponseStruct res = {amount, ml, comx, comy, comqx.toString(), comqy.toString(), r, rj, dt};
    std::string json = marshal_Res(&res);
    put_state(Response_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
    
    BigIntegerStruct ss = {sum.toString()};
    json = marshal_Int(&ss);
    put_state(Sum_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
    BigIntegerStruct x1 = {x.toString()};
    json = marshal_Int(&x1);
    put_state(X_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
    BigIntegerStruct x2 = {xj.toString()};
    json = marshal_Int(&x2);
    put_state(XJ_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
    
    return "ComqX: " + comqx.toString() + " ComqY: " + comqy.toString();
}

std::string Refund(std::string mlj, std::string res, shim_ctx_ptr_t ctx) {
    std::string hres = sha256(res);
    std::string Response_name(PREFIX + "Res" + SEP + hres + SEP);
    uint32_t res_bytes_len = 0;
    uint8_t res_bytes[MAX_VALUE_SIZE];
    get_state(Response_name.c_str(), res_bytes, sizeof(res_bytes), &res_bytes_len, ctx);
    
    if (res_bytes_len > 0) {
    	ResponseStruct re;
		unmarshal_Res(&re, (const char*)res_bytes, res_bytes_len);
		
		if (mlj.compare(re.ml) == 0) {
			return TRUE;
		}
		else {
			LOG_DEBUG("Materials list in donation feedback do not match.");
			return FALSE;
		}
	}
	else {
		LOG_DEBUG("Responses in donation feedback do not match.");
		return FALSE;
	}
}

std::string Audit(shim_ctx_ptr_t ctx) {
    std::string Sum_name = "Sum";		// Sum of amount
    uint32_t sum_bytes_len = 0;
    uint8_t sum_bytes[MAX_VALUE_SIZE];
    get_state(Sum_name.c_str(), sum_bytes, sizeof(sum_bytes), &sum_bytes_len, ctx);
    
    std::string X_name = "X";			// Sum of r
    uint32_t x_bytes_len = 0;
    uint8_t x_bytes[MAX_VALUE_SIZE];
    get_state(X_name.c_str(), x_bytes, sizeof(x_bytes), &x_bytes_len, ctx);
    
    std::string XJ_name = "XJ";			// Sum of rj
    uint32_t xj_bytes_len = 0;
    uint8_t xj_bytes[MAX_VALUE_SIZE];
    get_state(XJ_name.c_str(), xj_bytes, sizeof(xj_bytes), &xj_bytes_len, ctx);
	
    BigInteger sum, x, xj;
    if (sum_bytes_len == 0 && x_bytes_len == 0 && xj_bytes_len == 0) {
        sum = zero;
        x = zero;
        xj = zero;
    } 
    else {
        BigIntegerStruct a, b, c;
        unmarshal_Int(&a, (const char*)sum_bytes, sum_bytes_len);
        unmarshal_Int(&b, (const char*)x_bytes, x_bytes_len);
        unmarshal_Int(&c, (const char*)xj_bytes, xj_bytes_len);

        BigInteger d(a.num);
        BigInteger e(b.num);
        BigInteger f(c.num);
        sum = d;
        x = e;
        xj = f;
    }
	
    BigInteger ACominX, ACominY, AComoutX, AComoutY;
    std::string res_composite_key = PREFIX + "Res" + SEP;
    std::map<std::string, std::string> values;
    get_state_by_partial_composite_key(res_composite_key.c_str(), values, ctx);
    
    if (values.empty())
    {
        LOG_DEBUG("Donate_DoSC: No Distributes");
        return DONATE_NO_DISTRIBUTES;
    }
    else
    {
        for (auto u : values) {
            ResponseStruct res;
            unmarshal_Res(&res, u.second.c_str(), u.second.size());
            
            BigInteger cominx(res.comx);
            BigInteger cominy(res.comy);
            if (ACominX == zero && ACominY == zero) {
                ACominX = cominx;
                ACominY = cominy;
            } else {
                std::tie(ACominX, ACominY) = ecadd(ACominX, ACominY, cominx, cominy);
            }
            
            BigInteger comoutx(res.comqx);
            BigInteger comouty(res.comqy);
            if (AComoutX == zero && AComoutY == zero) {
                AComoutX = comoutx;
                AComoutY = comoutx;
            } else {
                std::tie(AComoutX, AComoutY) = ecadd(AComoutX, AComoutY, comoutx, comoutx);
            }
        }
    }
    
    BigInteger k(getRand());
    BigInteger r1(getRand());
    BigInteger r2(getRand());					// generate random numbers k, r1, r2
    BigInteger Tx1, Ty1, Tx2, Ty2;
    std::tie(Tx1, Ty1) = genProof(k, r1);
    std::tie(Tx2, Ty2) = genProof(k, r2);

    std::string message = "ACominX: " + ACominX.toString() + " ACominY: " + ACominY.toString() + " AComoutX: " + AComoutX.toString() + " AComoutY: " + AComoutY.toString();
	std::string commit = "Tx1: " + Tx1.toString() + " Ty1: " + Ty1.toString() + " Tx2: " + Tx2.toString() + " Ty2: " + Ty2.toString();
//	std::string message = "ACominX: " + getRand() + " ACominY: " + getRand() + " AComoutX: " + getRand() + " AComoutY: " + getRand();
//	std::string commit = "Tx1: " + getRand() + " Ty1: " + getRand() + " Tx2: " + getRand() + " Ty2: " + getRand();
    BigInteger c(sha256(Tx1.toString() + Ty1.toString() + Tx2.toString() + Ty2.toString()));
    BigInteger z = c * sum + k;
    BigInteger zr1 = c * x + r1;
    BigInteger zr2 = c * xj + r2;
    
    std::string reponse = "z: " + z.toString() + " zr1: " + zr1.toString() + " zr2: " + zr2.toString();
    return message + " | " + commit + " | " + reponse;
}

std::string Clear(shim_ctx_ptr_t ctx) {
    std::string Del_name = "Sum";
    del_state(Del_name.c_str(), ctx);
    Del_name = "X";
    del_state(Del_name.c_str(), ctx);
    Del_name = "XJ";
    del_state(Del_name.c_str(), ctx);
    return OK;
}

int invoke(uint8_t* response, uint32_t max_response_len, uint32_t* actual_response_len, shim_ctx_ptr_t ctx) {
    LOG_DEBUG("DiSC: +++ Executing DiSC chaincode invocation +++");

    std::string function_name;
    std::vector<std::string> params;
    get_func_and_params(function_name, params, ctx);
    
    char msp_id[MAX_MSP_ID_LEN];
    char dn[MAX_DN_LEN];
    get_creator_name(msp_id, MAX_MSP_ID_LEN, dn, MAX_DN_LEN, ctx);
    std::string creator_name = std::string(msp_id) + dn;

    std::string result;
    if (function_name == "Distribute") {
        result = Distribute(params[0], params[1], params[2], params[3], params[4], params[5], params[6], ctx);
    } else if (function_name == "Refund") {
        result = Refund(params[0], params[1], ctx);
    } else if (function_name == "Audit") {
        result = Audit(ctx);
	} else if (function_name == "Clear") {
        result = Clear(ctx);
    } else {
        LOG_DEBUG("DoSC: RECEIVED UNKNOWN transaction '%s'", function_name);
        return -1;
    }

    int neededSize = result.size();
    if (max_response_len < neededSize) {
        LOG_DEBUG("DoSC: Response buffer too small");
        *actual_response_len = 0;
        return -1;
    }

    memcpy(response, result.c_str(), neededSize);
    *actual_response_len = neededSize;
    LOG_DEBUG("DoSC: Response: %s", result.c_str());
    LOG_DEBUG("DoSC: +++ Executing done +++");
    return 0;
}
