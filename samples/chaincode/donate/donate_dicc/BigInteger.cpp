#include "BigInteger.h"
#include <algorithm>
#include <cctype>

bool BigInteger::isValidDecimal(const std::string& numStr) const {
    size_t start = (numStr[0] == '-') ? 1 : 0;
    for (size_t i = start; i < numStr.size(); ++i) {
        if (!isdigit(numStr[i])) {
            return false;
        }
    }
    return true;
}

bool BigInteger::isValidHex(const std::string& numStr) const {
    size_t start = (numStr[0] == '-') ? 1 : 0;
    for (size_t i = start; i < numStr.size(); ++i) {
        char c = toupper(numStr[i]);
        if (!isdigit(c) && (c < 'A' || c > 'F')) {
            return false;
        }
    }
    return true;
}

void BigInteger::fromHexString(const std::string& numStr) {
    digits.clear();
    size_t start = (numStr[0] == '-') ? 1 : 0;
    BigInteger result("0");
    BigInteger sixteen("16");

    for (size_t i = start; i < numStr.size(); ++i) {
        char c = toupper(numStr[i]);
        int digit = (c >= 'A') ? (c - 'A' + 10) : (c - '0');
        result = result * sixteen + BigInteger(std::to_string(digit));
    }

    digits = result.digits;
    if (digits.empty()) digits.push_back(0);
}

void BigInteger::fromDecimalString(const std::string& numStr) {
    size_t start = (numStr[0] == '-') ? 1 : 0;
    for (size_t i = start; i < numStr.size(); ++i) {
        digits.push_back(numStr[i] - '0');
    }
    std::reverse(digits.begin(), digits.end());
}

BigInteger::BigInteger(const std::string& numStr) {
    isNegative = false;

    if (numStr.size() >= 2 && numStr[0] == '0' && toupper(numStr[1]) == 'X') {
        if (!isValidHex(numStr.substr(2))) {
            std::string errorMsg = "Invalid hexadecimal string: " + numStr;
            for (char c : numStr.substr(2)) {
                if (islower(c)) {
                    errorMsg += " (Hint: Use uppercase letters A-F instead of a-f)";
                    break;
                }
            }
            throw std::invalid_argument(errorMsg);
        }
        isNegative = (numStr[0] == '-');
        fromHexString(numStr.substr(2));
    } else {
        if (!isValidDecimal(numStr)) {
            throw std::invalid_argument("Invalid decimal string: " + numStr);
        }
        isNegative = (numStr[0] == '-');
        fromDecimalString(numStr);
    }
}

std::string BigInteger::toString() const {
    std::string result;
    if (isNegative) result += '-';
    for (int i = digits.size() - 1; i >= 0; --i) {
        result += ('0' + digits[i]);
    }
    return result;
}

std::string BigInteger::toBinary() const {
    BigInteger num = *this;
    std::string binary;

    if (num == BigInteger("0")) {
        return "0";
    }

    while (num > BigInteger("0")) {
        binary = (num % BigInteger("2")).toString() + binary;
        num = num / BigInteger("2");
    }

    return binary;
}

BigInteger BigInteger::fromBinary(const std::string& binary) {
    BigInteger result("0");
    BigInteger power("1");

    for (int i = binary.size() - 1; i >= 0; --i) {
        if (binary[i] == '1') {
            result = result + power;
        }
        power = power * BigInteger("2");
    }

    return result;
}

BigInteger BigInteger::operator+(const BigInteger& other) const {
    BigInteger result;
    result.digits.clear();
    int carry = 0;
    int maxLen = std::max(digits.size(), other.digits.size());

    for (int i = 0; i < maxLen; ++i) {
        int sum = carry;
        if (i < digits.size()) sum += digits[i];
        if (i < other.digits.size()) sum += other.digits[i];
        result.digits.push_back(sum % 10);
        carry = sum / 10;
    }

    if (carry) result.digits.push_back(carry);
    return result;
}

BigInteger BigInteger::operator-(const BigInteger& other) const {
    BigInteger result;
    result.digits.clear();
    int borrow = 0;
    int maxLen = std::max(digits.size(), other.digits.size());

    for (int i = 0; i < maxLen; ++i) {
        int diff = borrow;
        if (i < digits.size()) diff += digits[i];
        if (i < other.digits.size()) diff -= other.digits[i];
        if (diff < 0) {
            diff += 10;
            borrow = -1;
        } else {
            borrow = 0;
        }
        result.digits.push_back(diff);
    }

    while (result.digits.size() > 1 && result.digits.back() == 0) {
        result.digits.pop_back();
    }

    return result;
}

BigInteger BigInteger::operator*(const BigInteger& other) const {
    BigInteger result;
    result.digits.resize(digits.size() + other.digits.size(), 0);

    for (int i = 0; i < digits.size(); ++i) {
        int carry = 0;
        for (int j = 0; j < other.digits.size(); ++j) {
            int product = digits[i] * other.digits[j] + result.digits[i + j] + carry;
            result.digits[i + j] = product % 10;
            carry = product / 10;
        }
        if (carry) result.digits[i + other.digits.size()] += carry;
    }

    while (result.digits.size() > 1 && result.digits.back() == 0) {
        result.digits.pop_back();
    }

    return result;
}

BigInteger BigInteger::operator/(const BigInteger& other) const {
    if (other == BigInteger("0")) {
        throw std::runtime_error("Division by zero");
    }

    BigInteger result;
    BigInteger remainder = *this;
    BigInteger divisor = other;

    while (remainder > divisor || remainder == divisor) {
        BigInteger temp = divisor;
        BigInteger quotient("1");

        while (remainder > (temp * BigInteger("10")) || remainder == (temp * BigInteger("10"))) {
            temp = temp * BigInteger("10");
            quotient = quotient * BigInteger("10");
        }

        while (remainder > temp || remainder == temp) {
            remainder = remainder - temp;
            result = result + quotient;
        }
    }

    return result;
}

BigInteger BigInteger::operator%(const BigInteger& other) const {
    if (other == BigInteger("0")) {
        throw std::runtime_error("Division by zero");
    }

    BigInteger remainder = *this;
    BigInteger divisor = other;

    while (remainder > divisor || remainder == divisor) {
        BigInteger temp = divisor;

        while (remainder > (temp * BigInteger("10")) || remainder == (temp * BigInteger("10"))) {
            temp = temp * BigInteger("10");
        }

        while (remainder > temp || remainder == temp) {
            remainder = remainder - temp;
        }
    }

    return remainder;
}

BigInteger BigInteger::operator&(const BigInteger& other) const {
    std::string binaryA = this->toBinary();
    std::string binaryB = other.toBinary();

    int maxLen = std::max(binaryA.size(), binaryB.size());
    binaryA.insert(0, maxLen - binaryA.size(), '0');
    binaryB.insert(0, maxLen - binaryB.size(), '0');

    std::string resultBinary;
    for (int i = 0; i < maxLen; ++i) {
        if (binaryA[i] == '1' && binaryB[i] == '1') {
            resultBinary += '1';
        } else {
            resultBinary += '0';
        }
    }

    return fromBinary(resultBinary);
}

bool BigInteger::operator<(const BigInteger& other) const {
    if (isNegative != other.isNegative) {
        return isNegative;
    }

    if (digits.size() != other.digits.size()) {
        return (digits.size() < other.digits.size()) ^ isNegative;
    }

    for (int i = digits.size() - 1; i >= 0; --i) {
        if (digits[i] != other.digits[i]) {
            return (digits[i] < other.digits[i]) ^ isNegative;
        }
    }

    return false;
}

bool BigInteger::operator==(const BigInteger& other) const {
    if (isNegative != other.isNegative || digits.size() != other.digits.size()) {
        return false;
    }

    for (int i = 0; i < digits.size(); ++i) {
        if (digits[i] != other.digits[i]) {
            return false;
        }
    }

    return true;
}

bool BigInteger::operator>(const BigInteger& other) const {
    return !(*this < other) && !(*this == other);
}

bool BigInteger::operator!=(const BigInteger& other) const {
    return !(*this == other);
}
