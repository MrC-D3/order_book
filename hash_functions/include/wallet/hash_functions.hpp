#include <string>
#include <cmath>
#include <vector>


// Limited if used for Hash Tables because H is also the size of the table and 
//  you would want to choose the size freely, while the divisor should respect
//  some rules to avoid clustering.
class DivisionHashing
{
  public:
    DivisionHashing(int H)
    : m_H{H}
    {}

    int operator()(int input)
    {
        return input % m_H;
    }

  private:
    int m_H;
};

// A.k.a. Knuth Hashing. Not machine efficient because of floating point. 
class KnuthHashing
{
  public:
    // By default, use the Golden Ration for A = phi-1 (e.g. the fraction part).
    //  Where phi: (a+b):a = a:b.
    KnuthHashing(int H, double A = 0.6180339887498948)
    : H{H}, A{A}
    {}

    int operator()(int input)
    {
        return std::floor (H * (input * A % 1));
    }

  private:
    int H;
    double A;
};

class MultiplicativeHashing
{

};

// 
class UniversalHashing
{
  public:
    UniversalHashing(int k = 2)
    : k(k)
    {
        for(auto i = 0; i < k; i++)
        {
            coefficients[i] = rand();
        }
    }

    int operator()(int key)
    {
        // ((ax+b) % p) % H
        int hash = 0;

        // Compute polynomial value.
        for(auto i = 0; i < k; i++)
        {
            hash += coefficients[i] * std::pow(key, i);
        }
        // Finite Field property.
        hash %= p;
        // Bucket index.
        hash %= H;

        return hash;
    }

  private:
    int k;
    std::vector<int> coefficients;
    int H; // Also called m, the size of the hash table.
    int p{2'147'483'647}; // Greatest prime number in 32 bits.
};

// 
class FNVHashing
{
  public:
    uint32_t operator()(const std::string& message)
    {
        // Per ogni byte in input:
        // - XOR col valore corrente (si parte da offset basis);
        // - moltiplichi per la costante prima.
        uint32_t result = offset_basis;
        for (auto byte : message)
        {
            // Cast implicito da char (uint8) a uint32.
            result ^= byte;
            result *= prime;
        }

        return result;
    }

  private:
    // Valori scelti perchè hanno buone proprietà di distribuzione.
    // Esistono anche le versioni a 64-bit.
    const uint32_t offset_basis{2166136261u};
    const uint32_t prime{16777619u};
};

//
class xxHashing
{

}

class xxHash64 {
private:
    // xxHash64 constants
    static constexpr uint64_t PRIME64_1 = 0x9E3779B185EBCA87ULL;
    static constexpr uint64_t PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
    static constexpr uint64_t PRIME64_3 = 0x165667B19E3779F9ULL;
    static constexpr uint64_t PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
    static constexpr uint64_t PRIME64_5 = 0x27D4EB2F165667C5ULL;

    // Utility functions
    static uint64_t rotl64(uint64_t x, int r) {
        return (x << r) | (x >> (64 - r));
    }

    static uint64_t read64(const void* ptr) {
        uint64_t val;
        std::memcpy(&val, ptr, 8);
        return val;
    }

    static uint32_t read32(const void* ptr) {
        uint32_t val;
        std::memcpy(&val, ptr, 4);
        return val;
    }

    static uint8_t read8(const void* ptr) {
        return *static_cast<const uint8_t*>(ptr);
    }

    // Process one 64-bit value
    static uint64_t round(uint64_t acc, uint64_t input) {
        acc += input * PRIME64_2;
        acc = rotl64(acc, 31);
        acc *= PRIME64_1;
        return acc;
    }

    // Merge accumulator
    static uint64_t mergeRound(uint64_t acc, uint64_t val) {
        val = round(0, val);
        acc ^= val;
        acc = acc * PRIME64_1 + PRIME64_4;
        return acc;
    }

    // Final avalanche mixing
    static uint64_t avalanche(uint64_t h64) {
        h64 ^= h64 >> 33;
        h64 *= PRIME64_2;
        h64 ^= h64 >> 29;
        h64 *= PRIME64_3;
        h64 ^= h64 >> 32;
        return h64;
    }

public:
    static uint64_t hash(const void* input, size_t len, uint64_t seed = 0) {
        const uint8_t* p = static_cast<const uint8_t*>(input);
        const uint8_t* const bEnd = p + len;
        uint64_t h64;

        if (len >= 32) {
            // Initialize accumulators
            uint64_t v1 = seed + PRIME64_1 + PRIME64_2;
            uint64_t v2 = seed + PRIME64_2;
            uint64_t v3 = seed + 0;
            uint64_t v4 = seed - PRIME64_1;

            // Process 32-byte stripes
            do {
                v1 = round(v1, read64(p)); p += 8;
                v2 = round(v2, read64(p)); p += 8;
                v3 = round(v3, read64(p)); p += 8;
                v4 = round(v4, read64(p)); p += 8;
            } while (p <= bEnd - 32);

            // Merge accumulators
            h64 = rotl64(v1, 1) + rotl64(v2, 7) + rotl64(v3, 12) + rotl64(v4, 18);
            h64 = mergeRound(h64, v1);
            h64 = mergeRound(h64, v2);
            h64 = mergeRound(h64, v3);
            h64 = mergeRound(h64, v4);
        } else {
            // Small input, use simple algorithm
            h64 = seed + PRIME64_5;
        }

        h64 += static_cast<uint64_t>(len);

        // Process remaining bytes
        while (p + 8 <= bEnd) {
            uint64_t k1 = round(0, read64(p));
            h64 ^= k1;
            h64 = rotl64(h64, 27) * PRIME64_1 + PRIME64_4;
            p += 8;
        }

        if (p + 4 <= bEnd) {
            h64 ^= static_cast<uint64_t>(read32(p)) * PRIME64_1;
            h64 = rotl64(h64, 23) * PRIME64_2 + PRIME64_3;
            p += 4;
        }

        while (p < bEnd) {
            h64 ^= static_cast<uint64_t>(read8(p)) * PRIME64_5;
            h64 = rotl64(h64, 11) * PRIME64_1;
            p++;
        }

        return avalanche(h64);
    }

    // Convenience function for strings
    static uint64_t hash(const std::string& str, uint64_t seed = 0) {
        return hash(str.c_str(), str.length(), seed);
    }
};

//
class SHA256Hashing
{
  public:
    // Meglio ragionare su stringhe anche per i numeri altrimenti la loro 
    //  lunghezza - e quindi l'hash risultante - dipenderebbe dall'architettura.
    //  Es.: 255 può stare su 1/2/4/8 byte come intero, ma sempre su 3 byte come
    //  stringa.
    std::string operator()(const std::string& message) 
    {
        // Reset hash values
        H[0] = 0x6a09e667; H[1] = 0xbb67ae85; H[2] = 0x3c6ef372; H[3] = 0xa54ff53a;
        H[4] = 0x510e527f; H[5] = 0x9b05688c; H[6] = 0x1f83d9ab; H[7] = 0x5be0cd19;
        
        // Convert message to bytes
        std::vector<uint8_t> data(message.begin(), message.end());
        uint64_t originalLength = data.size() * 8; // Length in bits
        
        // Padding: append single '1' bit
        data.push_back(0x80);
        
        // Pad with zeros until length ≡ 448 (mod 512)
        while (data.size() % 64 != 56) 
        {
            data.push_back(0x00);
        }
        
        // Append original length as 64-bit big-endian integer
        for (int i = 7; i >= 0; i--) 
        {
            data.push_back((originalLength >> (i * 8)) & 0xFF);
        }
        
        // Process message in 512-bit blocks
        for (size_t i = 0; i < data.size(); i += 64) 
        {
            processBlock(&data[i]);
        }
        
        // Convert hash to hex string
        std::stringstream ss;
        for (int i = 0; i < 8; i++) 
        {
            ss << std::hex << std::setfill('0') << std::setw(8) << H[i];
        }
        
        return ss.str();
    }

  private:
    // SHA-256 constants definitions. The first 32 bits (MSBs) of:
    // - fractional parts of cube roots of first 64 primes;
    const uint32_t K[64] = 
    {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
    // - fractional parts of square roots of first 8 primes.
    uint32_t H[8] = 
    {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    /*
    ** Methods
    */
    // Right rotate function
    uint32_t rotr(uint32_t x, int n) 
    {
        return (x >> n) | (x << (32 - n));
    }

    // SHA-256 logical functions
    // Parametri scelti per massimizzare la non-linearità e far sì che se anche 
    //  un solo bit cambia, l'ouput cambi dratistacamente.
    uint32_t gamma0(uint32_t x) 
    {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }
    uint32_t gamma1(uint32_t x) 
    {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }
    uint32_t sigma0(uint32_t x) 
    {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }
    uint32_t sigma1(uint32_t x) 
    {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }
    uint32_t ch(uint32_t x, uint32_t y, uint32_t z) 
    {
        return (x & y) ^ (~x & z);
    }
    uint32_t maj(uint32_t x, uint32_t y, uint32_t z) 
    {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    // Process a single 512-bit block
    void processBlock(const uint8_t* block) 
    {
        uint32_t W[64];
        
        // Prepare Message Schedule
        //  From 64 bytes to 16 words of 4 bytes.
        //  These 16 words ends in an array of 64 words.
        for (int t = 0; t < 16; t++) 
        {
            W[t] = (block[t * 4] << 24) | (block[t * 4 + 1] << 16) | 
                   (block[t * 4 + 2] << 8) | block[t * 4 + 3];
        }
        
        // Message Schedule, from 16 to 64 bytes: SHA-256 recurrence relation to
        //  cause Avalance Effect.
        for (int t = 16; t < 64; t++) 
        {
            W[t] = gamma1(W[t - 2]) + W[t - 7] + gamma0(W[t - 15]) + W[t - 16];
        }
        
        // Initialize working variables
        uint32_t a = H[0], b = H[1], c = H[2], d = H[3];
        uint32_t e = H[4], f = H[5], g = H[6], h = H[7];
        
        // Main compression loop
        for (int t = 0; t < 64; t++) 
        {
            uint32_t T1 = h + sigma1(e) + ch(e, f, g) + K[t] + W[t];
            uint32_t T2 = sigma0(a) + maj(a, b, c);
            
            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;
        }
        
        // Update hash values
        H[0] += a; H[1] += b; H[2] += c; H[3] += d;
        H[4] += e; H[5] += f; H[6] += g; H[7] += h;
    }
};
