#include <algorithm>
#include <utility>
#include "libsrp6.h"
#include <openssl/evp.h>
#include <iostream>
#include <vector>
#include <cstring>

extern "C" void test_srp6(
        const char* N_str,
        const char* g_str,
        const char* B_str,
        const char* s_str,
        const char* username_str,
        const char* password_str,
        const char* a_str
) {

    std::cout << "username=" << username_str << " | " << strlen(username_str) << std::endl;
    std::cout << "password=" << password_str << std::endl;

    BigNum N(reinterpret_cast<const unsigned char *>(N_str), 32);
    BigNum g(reinterpret_cast<const unsigned char *>(g_str), 1);
    BigNum s(reinterpret_cast<const unsigned char *>(s_str), 32);
    BigNum B(reinterpret_cast<const unsigned char *>(B_str), 32);
    BigNum a(reinterpret_cast<const unsigned char *>(a_str), 19);

    std::string username(username_str, strlen(username_str));
    std::string password(password_str, strlen(password_str));

    SRP6 client(N, g, s, B, username, password, a);
    client.calculate_session_key();
}

SRP6::SRP6(BigNum N, BigNum g, BigNum s, BigNum B, std::string username, std::string password):
N(N), g(g), s(s), B(B), username(std::move(username)), password(std::move(password))
{
    std::transform(this->username.begin(), this->username.end(), this->username.begin(), ::toupper);
    std::transform(this->password.begin(), this->password.end(), this->password.begin(), ::toupper);
    k.from_dec("3");

    calculate_private_ephemeral();
    std::cout << "a=" << a.to_hex() << std::endl;
    calculate_public_ephemeral();
    std::cout << "A=" << A.to_hex() << std::endl;
    calculate_x();
    std::cout << "x=" << x.to_hex() << std::endl;
    calculate_u();
    std::cout << "u=" << u.to_hex() << std::endl;
    calculate_S();
    std::cout << "S=" << S.to_hex() << std::endl;
}

SRP6::SRP6(BigNum N, BigNum g, BigNum s, BigNum B, std::string username, std::string password, BigNum a):
N(N), g(g), s(s), B(B), username(std::move(username)), password(std::move(password)), a(a) {
    std::transform(this->username.begin(), this->username.end(), this->username.begin(), ::toupper);
    std::transform(this->password.begin(), this->password.end(), this->password.begin(), ::toupper);
    k.from_dec("3");

    std::cout << "a=" << a.to_hex() << std::endl;
    calculate_public_ephemeral();
    std::cout << "A=" << A.to_hex() << std::endl;
    calculate_x();
    std::cout << "x=" << x.to_hex() << std::endl;
    calculate_u();
    std::cout << "u=" << u.to_hex() << std::endl;
    calculate_S();
    std::cout << "S=" << S.to_hex() << std::endl;
}

BigNum SRP6::calculate_session_key() {
    calculate_interleaved();
    return K;
}

BigNum SRP6::calculate_proof() {
    EVP_MD_CTX* digest = EVP_MD_CTX_new();
    auto* result = new unsigned char[20];

    unsigned char *xor_hash = calculate_xor_hash();
    unsigned char *username_hash = calculate_username_hash();

    EVP_DigestInit(digest, EVP_sha1());
    EVP_DigestUpdate(digest, xor_hash, 20);
    EVP_DigestUpdate(digest, username_hash, 20);
    EVP_DigestUpdate(digest, s.to_bin(32), 32);
    EVP_DigestUpdate(digest, A.to_bin(32), 32);
    EVP_DigestUpdate(digest, B.to_bin(32), 32);
    EVP_DigestUpdate(digest, K.to_bin(40), 40);
    EVP_DigestFinal(digest, result, nullptr);

    M.from_bin(result, 20);

    return M;
}

void SRP6::calculate_x() {
    EVP_MD_CTX* digest = EVP_MD_CTX_new();
    auto* result = new unsigned char[20];

    const auto* bytes = reinterpret_cast<const unsigned char*>(username.data());
    std::cout << "[C++] bytes=" << bytes << std::endl;
    for (size_t i = 0; i < username.length(); ++i) {
        std::cout << static_cast<int>(bytes[i]) << " ";
    }
    std::cout << std::endl;

    std::vector<uint8_t> myVector = {1, 2, 3};
    EVP_DigestInit(digest, EVP_sha1());
    EVP_DigestUpdate(digest, myVector.data(), 3);

    EVP_DigestFinal(digest, result, nullptr);

    BigNum p1;
    p1.from_bin(result, 20);

    std::cout << "[C++] p1=" << p1.to_hex() << std::endl;
    std::cout << "[C++] p1=" << p1.to_dec() << std::endl;

    std::cout << "username=" << username << std::endl;
    std::cout << "password=" << password << std::endl;
    std::cout << "s.to_bin(32)=" << s.to_bin(32) << std::endl;

    EVP_DigestInit(digest, EVP_sha1());
    EVP_DigestUpdate(digest, username.c_str(), username.length());
    EVP_DigestUpdate(digest, ":", 1);
    EVP_DigestUpdate(digest, password.c_str(), password.length());

    EVP_DigestFinal(digest, result, nullptr);

    BigNum p;
    p.from_bin(result, 20);

    EVP_DigestInit(digest, EVP_sha1());
    EVP_DigestUpdate(digest, s.to_bin(32), 32);
    EVP_DigestUpdate(digest, p.to_bin(20), 20);

    EVP_DigestFinal(digest, result, nullptr);

    x.from_bin(result, 20);

    EVP_MD_CTX_free(digest);
}

void SRP6::calculate_u() {
    EVP_MD_CTX* digest = EVP_MD_CTX_new();
    auto* result = new unsigned char[20];

    EVP_DigestInit(digest, EVP_sha1());
    EVP_DigestUpdate(digest, A.to_bin(32), 32);
    EVP_DigestUpdate(digest, B.to_bin(32), 32);

    EVP_DigestFinal(digest, result, nullptr);
}

void SRP6::calculate_public_ephemeral() {
    std::cout << "[C++]g=" << g.to_dec() << std::endl;
    std::cout << "[C++]a=" << a.to_dec() << std::endl;
    std::cout << "[C++]N=" << N.to_dec() << std::endl;
    A = g.mod_exp(a, N);
    std::cout << "[C++]A=" << A.to_dec() << std::endl;
}

void SRP6::calculate_private_ephemeral() {
    a.randomize(19);
}

void SRP6::calculate_S() {
    S = (B - (g.mod_exp(x, N) * k)).mod_exp(a + (u * x), N);
}

void SRP6::calculate_interleaved() {
    EVP_MD_CTX* digest = EVP_MD_CTX_new();
    auto* result = new unsigned char[20];

    unsigned char* S_bytes = S.to_bin(32);
    auto* odd = new unsigned char[16];
    auto* even = new unsigned char[16];
    auto* session_key = new unsigned char[40];

    for(int i = 0; i < 16; ++i){
        odd[i] = S_bytes[i * 2];
        even[i] = S_bytes[i * 2 + 1];
    }

    EVP_DigestInit(digest, EVP_sha1());
    EVP_DigestUpdate(digest, odd, 16);
    EVP_DigestFinal(digest, result, nullptr);

    std::copy(result, result + 20, session_key);

    EVP_DigestInit(digest, EVP_sha1());
    EVP_DigestUpdate(digest, odd, 16);
    EVP_DigestFinal(digest, result, nullptr);

    std::copy(result, result + 20, session_key + 20);

    K.from_bin(session_key, 40);

    delete[] odd;
    delete[] even;
    delete[] session_key;
}

unsigned char *SRP6::calculate_xor_hash() {
    EVP_MD_CTX* digest = EVP_MD_CTX_new();

    auto* N_hash = new unsigned char[20];
    auto* g_hash = new unsigned char[20];

    EVP_DigestInit(digest, EVP_sha1());
    EVP_DigestUpdate(digest, N.to_bin(32), 32);
    EVP_DigestFinal(digest, N_hash, nullptr);

    EVP_DigestInit(digest, EVP_sha1());
    EVP_DigestUpdate(digest, g.to_bin(1), 1);
    EVP_DigestFinal(digest, g_hash, nullptr);

    for(int i = 0; i < 20; ++i){
        N_hash[i] = N_hash[i] ^ g_hash[i];
    }

    return N_hash;
}

unsigned char *SRP6::calculate_username_hash() {
    EVP_MD_CTX* digest = EVP_MD_CTX_new();
    auto* result = new unsigned char[20];

    EVP_DigestInit(digest, EVP_sha1());
    EVP_DigestUpdate(digest, username.c_str(), username.length());
    EVP_DigestFinal(digest, result, nullptr);

    return result;
}