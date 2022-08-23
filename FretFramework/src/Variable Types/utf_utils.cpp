//==================================================================================================
//  File:       unicode_utils.cpp
//
//  Copyright (c) 2018 Bob Steagall and KEWB Computing, All Rights Reserved
//==================================================================================================
//
#include "utf_utils.h"
#include <cstdio>

#if defined KEWB_PLATFORM_LINUX
    #include <emmintrin.h>
    #include <immintrin.h>
    #include <xmmintrin.h>
#elif defined KEWB_PLATFORM_WINDOWS
    #include <intrin.h>
#endif

namespace uu {
//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a sequence of UTF-32 code points.
///
/// \details
///     This static member function reads an input sequence of UTF-8 code units and converts
///     it to an output sequence of UTF-32 code points.  It performs conversion by traversing
///     the DFA without any optimizations using the `AdvanceWithBigTable` member function to
///     read and convert input.
///
/// \param pSrc
///     A non-null pointer defining the beginning of the code unit input range.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the code unit input range.
/// \param pDst
///     A non-null pointer defining the beginning of the code point output range.
///
/// \returns
///     If successful, the number of UTF-32 code points written; otherwise -1 is returned to
///     indicate an error was encountered.
//--------------------------------------------------------------------------------------------------
//
KEWB_ALIGN_FN std::ptrdiff_t
UtfUtils::BasicBigTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept
{
    char32_t*   pDstOrig = pDst;
    char32_t    cdpt;

    while (pSrc < pSrcEnd)
    {
        if (AdvanceWithBigTable(pSrc, pSrcEnd, cdpt) != ERR)
        {
            *pDst++ = cdpt;
        }
        else
        {
            return -1;
        }
    }

    return pDst - pDstOrig;
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a sequence of UTF-32 code points.
///
/// \details
///     This static member function reads an input sequence of UTF-8 code units and converts
///     it to an output sequence of UTF-32 code points.  It uses the DFA to perform non-ascii
///     code-unit sequence conversions, but optimizes by checking for ASCII code units and
///     converting them directly to code points.  It uses the `AdvanceWithBigTable` member
///     function to read and convert input.
///
/// \param pSrc
///     A non-null pointer defining the beginning of the code unit input range.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the code unit input range.
/// \param pDst
///     A non-null pointer defining the beginning of the code point output range.
///
/// \returns
///     If successful, the number of UTF-32 code points written; otherwise -1 is returned to
///     indicate an error was encountered.
//--------------------------------------------------------------------------------------------------
//
KEWB_ALIGN_FN std::ptrdiff_t
UtfUtils::FastBigTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept
{
    char32_t*   pDstOrig = pDst;
    char32_t    cdpt;

    while (pSrc < pSrcEnd)
    {
        if (*pSrc < 0x80)
        {
            *pDst++ = *pSrc++;
        }
        else
        {
            if (AdvanceWithBigTable(pSrc, pSrcEnd, cdpt) != ERR)
            {
                *pDst++ = cdpt;
            }
            else
            {
                return -1;
            }
        }
    }

    return pDst - pDstOrig;
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a sequence of UTF-32 code points.
///
/// \details
///     This static member function reads an input sequence of UTF-8 code units and converts
///     it to an output sequence of UTF-32 code points.  It uses the DFA to perform non-ascii
///     code-unit sequence conversions, but optimizes by converting contiguous sequences of
///     ASCII code units using SSE intrinsics.  It uses the `AdvanceWithBigTable` member
///     function to read and convert input.
///
/// \param pSrc
///     A non-null pointer defining the beginning of the code unit input range.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the code unit input range.
/// \param pDst
///     A non-null pointer defining the beginning of the code point output range.
///
/// \returns
///     If successful, the number of UTF-32 code points written; otherwise -1 is returned to
///     indicate an error was encountered.
//--------------------------------------------------------------------------------------------------
//
KEWB_ALIGN_FN std::ptrdiff_t
UtfUtils::SseBigTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept
{
    char32_t*   pDstOrig = pDst;
    char32_t    cdpt;

    while (pSrc < (pSrcEnd - sizeof(__m128i)))
    {
        if (*pSrc < 0x80)
        {
            ConvertAsciiWithSse(pSrc, pDst);
        }
        else
        {
            if (AdvanceWithBigTable(pSrc, pSrcEnd, cdpt) != ERR)
            {
                *pDst++ = cdpt;
            }
            else
            {
                return -1;
            }
        }
    }

    while (pSrc < pSrcEnd)
    {
        if (*pSrc < 0x80)
        {
            *pDst++ = *pSrc++;
        }
        else
        {
            if (AdvanceWithBigTable(pSrc, pSrcEnd, cdpt) != ERR)
            {
                *pDst++ = cdpt;
            }
            else
            {
                return -1;
            }
        }
    }

    return pDst - pDstOrig;
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a sequence of UTF-32 code points.
///
/// \details
///     This static member function reads an input sequence of UTF-8 code units and converts
///     it to an output sequence of UTF-32 code points.  It performs conversion by traversing
///     the DFA without any optimizations using the `AdvanceWithSmallTable` member function to
///     read and convert input.
///
/// \param pSrc
///     A non-null pointer defining the beginning of the code unit input range.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the code unit input range.
/// \param pDst
///     A non-null pointer defining the beginning of the code point output range.
///
/// \returns
///     If successful, the number of UTF-32 code points written; otherwise -1 is returned to
///     indicate an error was encountered.
//--------------------------------------------------------------------------------------------------
//
KEWB_ALIGN_FN std::ptrdiff_t
UtfUtils::BasicSmallTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept
{
    char32_t*   pDstOrig = pDst;
    char32_t    cdpt;

    while (pSrc < pSrcEnd)
    {
        if (AdvanceWithSmallTable(pSrc, pSrcEnd, cdpt) != ERR)
        {
            *pDst++ = cdpt;
        }
        else
        {
            return -1;
        }
    }

    return pDst - pDstOrig;
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a sequence of UTF-32 code points.
///
/// \details
///     This static member function reads an input sequence of UTF-8 code units and converts
///     it to an output sequence of UTF-32 code points.  It uses the DFA to perform non-ascii
///     code-unit sequence conversions, but optimizes by checking for ASCII code units and
///     converting them directly to code points.  It uses the `AdvanceWithSmallTable` member
///     function to read and convert input.
///
/// \param pSrc
///     A non-null pointer defining the beginning of the code unit input range.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the code unit input range.
/// \param pDst
///     A non-null pointer defining the beginning of the code point output range.
///
/// \returns
///     If successful, the number of UTF-32 code points written; otherwise -1 is returned to
///     indicate an error was encountered.
//--------------------------------------------------------------------------------------------------
//
KEWB_ALIGN_FN std::ptrdiff_t
UtfUtils::FastSmallTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept
{
    char32_t*   pDstOrig = pDst;
    char32_t    cdpt;

    while (pSrc < pSrcEnd)
    {
        if (*pSrc < 0x80)
        {
            *pDst++ = *pSrc++;
        }
        else
        {
            if (AdvanceWithSmallTable(pSrc, pSrcEnd, cdpt) != ERR)
            {
                *pDst++ = cdpt;
            }
            else
            {
                return -1;
            }
        }
    }

    return pDst - pDstOrig;
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of UTF-8 code units to a sequence of UTF-32 code points.
///
/// \details
///     This static member function reads an input sequence of UTF-8 code units and converts
///     it to an output sequence of UTF-32 code points.  It uses the DFA to perform non-ascii
///     code-unit sequence conversions, but optimizes by converting contiguous sequences of
///     ASCII code units using SSE intrinsics.  It uses the `AdvanceWithSmallTable` member
///     function to read and convert input.
///
/// \param pSrc
///     A non-null pointer defining the beginning of the code unit input range.
/// \param pSrcEnd
///     A non-null past-the-end pointer defining the end of the code unit input range.
/// \param pDst
///     A non-null pointer defining the beginning of the code point output range.
///
/// \returns
///     If successful, the number of UTF-32 code points written; otherwise -1 is returned to
///     indicate an error was encountered.
//--------------------------------------------------------------------------------------------------
//
KEWB_ALIGN_FN std::ptrdiff_t
UtfUtils::SseSmallTableConvert(unsigned char const* pSrc, unsigned char const* pSrcEnd, char32_t* pDst) noexcept
{
    char32_t*   pDstOrig = pDst;
    char32_t    cdpt;

    while (pSrc < (pSrcEnd - sizeof(__m128i)))
    {
        if (*pSrc < 0x80)
        {
            ConvertAsciiWithSse(pSrc, pDst);
        }
        else
        {
            if (AdvanceWithSmallTable(pSrc, pSrcEnd, cdpt) != ERR)
            {
                *pDst++ = cdpt;
            }
            else
            {
                return -1;
            }
        }
    }

    while (pSrc < pSrcEnd)
    {
        if (*pSrc < 0x80)
        {
            *pDst++ = *pSrc++;
        }
        else
        {
            if (AdvanceWithSmallTable(pSrc, pSrcEnd, cdpt) != ERR)
            {
                *pDst++ = cdpt;
            }
            else
            {
                return -1;
            }
        }
    }

    return pDst - pDstOrig;
}

//--------------------------------------------------------------------------------------------------
/// \brief  Converts a sequence of ASCII UTF-8 code units to a sequence of UTF-32 code points.
///
/// \details
///     This static member function uses SSE intrinsics to convert a register of ASCII code
///     units to four registers of equivalent UTF-32 code units.
///
/// \param pSrc
///     A reference to a non-null pointer defining the start of the code unit input range.
/// \param pDst
///     A reference to a non-null pointer defining the start of the code point output range.
//--------------------------------------------------------------------------------------------------
//
KEWB_FORCE_INLINE void
UtfUtils::ConvertAsciiWithSse(unsigned char const*& pSrc, char32_t*& pDst) noexcept
{
    __m128i     chunk, half, qrtr, zero;
    int32_t     mask, incr;

    zero  = _mm_set1_epi8(0);                           //- Zero out the interleave register
    chunk = _mm_loadu_si128((__m128i const*) pSrc);     //- Load a register with 8-bit bytes
    mask  = _mm_movemask_epi8(chunk);                   //- Determine which octets have high bit set

    half = _mm_unpacklo_epi8(chunk, zero);              //- Unpack bytes 0-7 into 16-bit words
    qrtr = _mm_unpacklo_epi16(half, zero);              //- Unpack words 0-3 into 32-bit dwords
    _mm_storeu_si128((__m128i*) pDst, qrtr);            //- Write to memory
    qrtr = _mm_unpackhi_epi16(half, zero);              //- Unpack words 4-7 into 32-bit dwords
    _mm_storeu_si128((__m128i*) (pDst + 4), qrtr);      //- Write to memory

    half = _mm_unpackhi_epi8(chunk, zero);              //- Unpack bytes 8-15 into 16-bit words
    qrtr = _mm_unpacklo_epi16(half, zero);              //- Unpack words 8-11 into 32-bit dwords
    _mm_storeu_si128((__m128i*) (pDst + 8), qrtr);      //- Write to memory
    qrtr = _mm_unpackhi_epi16(half, zero);              //- Unpack words 12-15 into 32-bit dwords
    _mm_storeu_si128((__m128i*) (pDst + 12), qrtr);     //- Write to memory

    //- If no bits were set in the mask, then all 16 code units were ASCII, and therefore
    //  both pointers are advanced by 16.
    //
    if (mask == 0)
    {
        pSrc += 16;
        pDst += 16;
    }

    //- Otherwise, the number of trailing (low-order) zero bits in the mask indicates the number
    //  of ASCII code units starting from the lowest byte address.
    else
    {
        incr  = GetTrailingZeros(mask);
        pSrc += incr;
        pDst += incr;
    }
}

//--------------------------------------------------------------------------------------------------
/// \brief  Returns the number of trailing 0-bits in an integer, starting with the least
///         significant bit.
///
/// \details
///     This static member function uses compiler intrinsics to determine the number of trailing
///     (i.e., low-order) zero bits in an int32_t parameter.  For example, an input value of 8
///     (0000 1000) would return a value of 3; an input value of 64 (0100 0000) would return a
///     value of 6.
///
/// \param x
///     An `int32_t` value whose number of trailing bits is to be determined.

/// \returns
///     the number of trailing zero bits, as an `int32_t`.
//--------------------------------------------------------------------------------------------------
//
#if defined KEWB_PLATFORM_LINUX  &&  (defined KEWB_COMPILER_CLANG  ||  defined KEWB_COMPILER_GCC)

    KEWB_FORCE_INLINE int32_t
    UtfUtils::GetTrailingZeros(int32_t x) noexcept
    {
        return  __builtin_ctz((unsigned int) x);
    }

#elif defined KEWB_PLATFORM_WINDOWS  &&  defined KEWB_COMPILER_MSVC

    KEWB_FORCE_INLINE int32_t
    UtfUtils::GetTrailingZeros(int32_t x) noexcept
    {
        unsigned long   indx;
        _BitScanForward(&indx, (unsigned long) x);
        return (int32_t) indx;
    }

#endif

}   //- Namespace uu
