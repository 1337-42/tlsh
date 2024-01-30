#pragma once

#include <array>
#include <memory>

#include "tlsh_version.h"

#define SLIDING_WND_SIZE 5

#define BUCKETS 256
#define Q_BITS 2 // 2 bits; quartile value 0, 1, 2, 3

#if NB_TLSH_BUCKETS == 256
#define TLS_BLOCK_GRANULARITY 64

#elif NB_TLSH_BUCKETS == 128
#define TLS_BLOCK_GRANULARITY 32

#elif NB_TLSH_BUCKETS == 48
#define TLS_BLOCK_GRANULARITY 12
#if TLSH_CHECKSUM == 3
#error "invalid bucket/checksum combination"
#endif

#else
#error "invalid bucket size"
#endif

#define EFF_BUCKETS NB_TLSH_BUCKETS
#define CODE_SIZE ((NB_TLSH_BUCKETS << 1) >> 3) // (NB_TLSH_BUCKETS * 2 bits) in bytes
#define TLSH_CHECKSUM_LEN (TLSH_CHECKSUM)
#define INTERNAL_TLSH_STRING_LEN (((2 + TLSH_CHECKSUM) + TLS_BLOCK_GRANULARITY) * 2)
#define TLSH_STRING_LEN (INTERNAL_TLSH_STRING_LEN)

class TlshImpl
{
public:
    TlshImpl();
    ~TlshImpl();

public:
    void
    update(const unsigned char *data, unsigned int len, int tlsh_option);
    void
    fast_update5(const unsigned char *data, unsigned int len, int tlsh_option);
    void
    final(int fc_cons_option = 0);
    void
    reset();
    const char *
    hash(int showvers) const;
    const char *
    hash(char *buffer,
        unsigned int bufSize,
        int showvers) const; // saves allocating hash string in TLSH instance - bufSize should be
                             // TLSH_STRING_LEN + 1
    int
    compare(const TlshImpl &other) const;
    int
    totalDiff(std::unique_ptr<TlshImpl> const &other, bool len_diff = true) const;
    int
    Lvalue();
    int
    Q1ratio();
    int
    Q2ratio();
    int
    Checksum(int k);
    int
    BucketValue(int bucket);
    int
    HistogramCount(int bucket);
    int
    fromTlshStr(const char *str);
    bool
    isValid() const
    {
        return lsh_code_valid;
    }

private:
    std::unique_ptr<u32[]> a_bucket;
    std::array<u8, SLIDING_WND_SIZE> slide_window;
    unsigned int data_len;

    struct lsh_bin_struct
    {
        unsigned char checksum[TLSH_CHECKSUM_LEN]; // 1 to 3 bytes
        unsigned char Lvalue;                      // 1 byte
        union
        {
#if defined(__SPARC) || defined(_AIX)
#pragma pack(1)
#endif
            unsigned char QB;
            struct
            {
#if defined(__SPARC) || defined(_AIX)
                unsigned char Q2ratio : 4;
                unsigned char Q1ratio : 4;
#else
                unsigned char Q1ratio : 4;
                unsigned char Q2ratio : 4;
#endif
            } QR;
        } Q;                               // 1 bytes
        unsigned char tmp_code[CODE_SIZE]; // 32/64 bytes
    } lsh_bin;

    mutable char *lsh_code; // allocated when hash() function without buffer is called - 70/134
                            // bytes or 74/138 bytes
    bool lsh_code_valid;    // true iff final() or fromTlshStr complete successfully
};
