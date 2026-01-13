#ifndef BIGINTEGER_H
#define BIGINTEGER_H

#include <string>
#include <vector>
#include <stdexcept>

class BigInteger {
private:
    std::vector<int> digits;
    bool isNegative;

    bool isValidDecimal(const std::string& numStr) const;
    bool isValidHex(const std::string& numStr) const;
    void fromHexString(const std::string& numStr);
    void fromDecimalString(const std::string& numStr);

public:
    BigInteger(const std::string& numStr = "0");
    
    std::string toString() const;
    std::string toBinary() const;
    static BigInteger fromBinary(const std::string& binary);

    BigInteger operator+(const BigInteger& other) const;
    BigInteger operator-(const BigInteger& other) const;
    BigInteger operator*(const BigInteger& other) const;
    BigInteger operator/(const BigInteger& other) const;
    BigInteger operator%(const BigInteger& other) const;
    BigInteger operator&(const BigInteger& other) const;

    bool operator<(const BigInteger& other) const;
    bool operator==(const BigInteger& other) const;
    bool operator>(const BigInteger& other) const;
    bool operator!=(const BigInteger& other) const;
};

#endif // BIGINTEGER_H
