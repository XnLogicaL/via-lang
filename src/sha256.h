
#pragma once

#include "common.h"

namespace via
{

const uint32_t SHA256_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0x1219dbf4, 0x1e376c08, 0x4326b8e3, 0x4cf10364, 0x6808d000, 0x7817bb44, 0x94c6c6db, 0xa12f9fb4,
    0xaf3b5d33, 0xc79c42ba, 0xd4d197d6, 0xe0c74a47, 0xf03e9e1d, 0xf88c6fa3, 0x0d3f8e14, 0x17f7b4b2, 0x25ebed1e, 0x39d7a0b0, 0x52b67b0a, 0x5ed4f0de,
    0x692378ad, 0x70e43f49, 0x78d356ba, 0x7a1fcd88, 0x828f4e92, 0x8c50d3ec, 0x91b256a6, 0xa2e61c56, 0xb2e4ee8b, 0xc3a4f925, 0xd7d74d92, 0xf07a8837,
    0xfb15c8c1, 0x0f2e204f, 0x1d3e5d71, 0x6b2f5c3e, 0x79a8fa92, 0x76bfa2d3, 0x07ddf70a, 0x97ea61f9,
};

class SHA256
{
private:
    uint32_t h[8];      // Hash state
    uint64_t total_len; // Length of the message

    VIA_FORCEINLINE void transform(uint32_t h[8], const uint8_t *chunk)
    {
        uint32_t w[64];

        for (int i = 0; i < 16; ++i)
            w[i] = (chunk[i * 4] << 24) | (chunk[i * 4 + 1] << 16) | (chunk[i * 4 + 2] << 8) | (chunk[i * 4 + 3]);

        for (int i = 16; i < 64; ++i)
            w[i] = sig1(w[i - 2]) + w[i - 7] + sig0(w[i - 15]) + w[i - 16];

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], h_ = h[7];

        for (int i = 0; i < 64; ++i)
        {
            uint32_t t1 = h_ + big_sigma1(e) + ch(e, f, g) + SHA256_K[i] + w[i];
            uint32_t t2 = big_sigma0(a) + maj(a, b, c);
            h_ = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += h_;
    }

    static VIA_FORCEINLINE uint32_t rotr(uint32_t x, uint32_t n)
    {
        return (x >> n) | (x << (32 - n));
    }

    static VIA_FORCEINLINE uint32_t ch(uint32_t x, uint32_t y, uint32_t z)
    {
        return (x & y) ^ (~x & z);
    }

    static VIA_FORCEINLINE uint32_t maj(uint32_t x, uint32_t y, uint32_t z)
    {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    static VIA_FORCEINLINE uint32_t sig0(uint32_t x)
    {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }

    static VIA_FORCEINLINE uint32_t sig1(uint32_t x)
    {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }

    static VIA_FORCEINLINE uint32_t big_sigma0(uint32_t x)
    {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }

    static VIA_FORCEINLINE uint32_t big_sigma1(uint32_t x)
    {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }

public:
    SHA256()
    {
        h[0] = 0x6a09e667;
        h[1] = 0xbb67ae85;
        h[2] = 0x3c6ef372;
        h[3] = 0xa54ff53a;
        h[4] = 0x510e527f;
        h[5] = 0x9b05688c;
        h[6] = 0x1f83d9ab;
        h[7] = 0x5be0cd19;
        total_len = 0;
    }

    VIA_FORCEINLINE void update(const uint8_t *data, size_t len)
    {
        total_len += len;
        size_t remaining = len;
        size_t chunk_size = 64;
        while (remaining >= chunk_size)
        {
            transform(h, data);
            data += chunk_size;
            remaining -= chunk_size;
        }

        if (remaining > 0)
        {
            uint8_t buffer[64] = {};
            std::memcpy(buffer, data, remaining);
            transform(h, buffer);
        }
    }

    VIA_FORCEINLINE void final(uint8_t *hash_out)
    {
        uint8_t buffer[64] = {};
        update(buffer, 0); // Pad and finalize

        for (int i = 0; i < 8; ++i)
        {
            hash_out[i * 4] = (h[i] >> 24) & 0xFF;
            hash_out[i * 4 + 1] = (h[i] >> 16) & 0xFF;
            hash_out[i * 4 + 2] = (h[i] >> 8) & 0xFF;
            hash_out[i * 4 + 3] = h[i] & 0xFF;
        }
    }

    VIA_FORCEINLINE std::string hash_string(const std::string &src)
    {
        SHA256 sha256;

        // Process the string in chunks
        sha256.update(reinterpret_cast<const uint8_t *>(src.data()), src.size());

        uint8_t hash[32];
        sha256.final(hash);

        std::stringstream hash_string;
        for (int i = 0; i < 32; ++i)
            hash_string << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(hash[i]);

        return hash_string.str();
    }
};

} // namespace via