#pragma once

#include <stdint.h>
// #include <immintrin.h>
#include <iostream>
#include <sstream>

template <unsigned int N>
struct VulpineBitSet
{
    struct templatePass{template<typename ...T> templatePass(T...) {}};

    // uint8_t bytes[(N-N%256)/8 + 32*((N%256 > 0))] = {0};
    uint8_t bytes[(N/8)+1] = {0};

    bool operator==(const VulpineBitSet &mask)
    {   
        bool res = true;

    /******* The compiler generated vectorized code runs 2.5 times faster
        for(uint i = 0; i < sizeof(bytes); i+=32)
        {
            __m256i a = _mm256_loadu_epi64(bytes+i);
            __m256i m = _mm256_loadu_epi64(mask.bytes+i);
            __m256i r = _mm256_and_si256(a, m);
            res &= _mm256_testc_si256(r, m);
        }
    *******/

        uint64_t *a = (uint64_t*)bytes;
        uint64_t *m = (uint64_t*)mask.bytes;
        for(uint32_t i = 0; i < sizeof(bytes)/8; i ++)
            res &= (a[i] & m[i]) == m[i];

        return res;
    };

    void setTrue(int i)
    {
        const int byte = i/8;
        bytes[byte] |= 1<<(i%8);
    };

    template<typename ... args>
    void setTrue(args&&... arg)
    {
        templatePass{(setTrue(arg), 0) ...};
    };

    template<typename ... args>
    VulpineBitSet(args&&... arg)
    {
        setTrue(arg ...);
    };


    std::string toStr()
    {
        std::stringstream ss;
        for(uint32_t i = 0; i < sizeof(bytes); i ++)
            for(uint32_t j = 0; j < 8; j ++)
                ss << (bytes[i]&(1<<j) ? '1' : '0');
        
        return ss.str(); 
    };
};


