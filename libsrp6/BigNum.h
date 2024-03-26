#pragma once

#include <openssl/bn.h>

class BigNum {
public:
    BigNum();
    BigNum(const unsigned char* data, int length);
    explicit BigNum(const char *data);

    BigNum from_bin(const unsigned char* data, int length);
    unsigned char* to_bin(int length);
    BigNum from_dec(const char *data);
    char *to_hex();

    BigNum operator+(const BigNum& b);
    BigNum operator-(const BigNum& b);
    BigNum operator*(const BigNum& b);
    BigNum operator/(const BigNum& b);
    BigNum mod_exp(const BigNum& b, const BigNum& c);
    void randomize(int bytes_amount);

    BigNum& operator=(const BigNum& b);
protected:
    BIGNUM* bn;
};