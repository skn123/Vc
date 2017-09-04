/*  This file is part of the Vc library. {{{
Copyright © 2017 Matthias Kretz <kretz@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the names of contributing organizations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

}}}*/

#include <array>

// is_conversion_undefined {{{1
/* implementation-defined
 * ======================
 * §4.7 p3 (integral conversions)
 *  If the destination type is signed, the value is unchanged if it can be represented in the
 *  destination type (and bit-field width); otherwise, the value is implementation-defined.
 *
 * undefined
 * =========
 * §4.9/1  (floating-point conversions)
 *   If the source value is neither exactly represented in the destination type nor between
 *   two adjacent destination values the result is undefined.
 *
 * §4.10/1 (floating-integral conversions)
 *  floating point type can be converted to integer type.
 *  The behavior is undefined if the truncated value cannot be
 *  represented in the destination type.
 *
 * §4.10/2
 *  integer can be converted to floating point type.
 *  If the value being converted is outside the range of values that can be represented, the
 *  behavior is undefined.
 */
template <typename To, typename From>
constexpr bool is_conversion_undefined_impl(From x, std::true_type)
{
    return x > static_cast<long double>(std::numeric_limits<To>::max()) ||
           x < static_cast<long double>(std::numeric_limits<To>::min());
}

template <typename To, typename From>
constexpr bool is_conversion_undefined_impl(From, std::false_type)
{
    return false;
}

template <typename To, typename From> constexpr bool is_conversion_undefined(From x)
{
    static_assert(std::is_arithmetic<From>::value,
                  "this overload is only meant for builtin arithmetic types");
    return is_conversion_undefined_impl<To, From>(
        x, std::integral_constant<bool, (std::is_floating_point<From>::value &&
                                         (std::is_integral<To>::value ||
                                          (std::is_floating_point<To>::value &&
                                           sizeof(From) > sizeof(To))))>());
}

static_assert(is_conversion_undefined<uint>(float(0x100000000LL)),
              "testing my expectations of is_conversion_undefined");
static_assert(!is_conversion_undefined<float>(llong(0x100000000LL)),
              "testing my expectations of is_conversion_undefined");

template <typename To, typename T, typename A>
inline Vc::simd_mask<T, A> is_conversion_undefined(const Vc::simd<T, A> &x)
{
    Vc::simd_mask<T, A> k = false;
    for (std::size_t i = 0; i < x.size(); ++i) {
        k[i] = is_conversion_undefined(x[i]);
    }
    return k;
}

//operators helpers  //{{{1
template <class T> constexpr T genHalfBits()
{
    return std::numeric_limits<T>::max() >> (std::numeric_limits<T>::digits / 2);
}
template <> constexpr long double genHalfBits<long double>() { return 0; }
template <> constexpr double genHalfBits<double>() { return 0; }
template <> constexpr float genHalfBits<float>() { return 0; }

template <class U, class T, class UU> constexpr U avoid_ub(UU x)
{
    return is_conversion_undefined<T>(U(x)) ? U(0) : U(x);
}

template <class U, class T, class UU> constexpr U avoid_ub2(UU x)
{
    return is_conversion_undefined<U>(x) ? U(0) : avoid_ub<U, T>(x);
}

// conversion test input data //{{{1
template <class U, class T>
static const std::array<U, 51> cvt_input_data = {{
    avoid_ub<U, T>(0xc0000080U),
    avoid_ub<U, T>(0xc0000081U),
    avoid_ub<U, T>(0xc0000082U),
    avoid_ub<U, T>(0xc0000084U),
    avoid_ub<U, T>(0xc0000088U),
    avoid_ub<U, T>(0xc0000090U),
    avoid_ub<U, T>(0xc00000A0U),
    avoid_ub<U, T>(0xc00000C0U),
    avoid_ub<U, T>(0xc000017fU),
    avoid_ub<U, T>(0xc0000180U),
    avoid_ub<U, T>(0x100000001LL),
    avoid_ub<U, T>(0x100000011LL),
    avoid_ub<U, T>(0x100000111LL),
    avoid_ub<U, T>(0x100001111LL),
    avoid_ub<U, T>(0x100011111LL),
    avoid_ub<U, T>(0x100111111LL),
    avoid_ub<U, T>(0x101111111LL),
    avoid_ub<U, T>(-0x100000001LL),
    avoid_ub<U, T>(-0x100000011LL),
    avoid_ub<U, T>(-0x100000111LL),
    avoid_ub<U, T>(-0x100001111LL),
    avoid_ub<U, T>(-0x100011111LL),
    avoid_ub<U, T>(-0x100111111LL),
    avoid_ub<U, T>(-0x101111111LL),
    avoid_ub<U, T>(std::numeric_limits<U>::min()),
    avoid_ub<U, T>(std::numeric_limits<U>::min() + 1),
    avoid_ub<U, T>(-1),
    avoid_ub<U, T>(-10),
    avoid_ub<U, T>(-100),
    avoid_ub<U, T>(-1000),
    avoid_ub<U, T>(-10000),
    avoid_ub<U, T>(0),
    avoid_ub<U, T>(1),
    avoid_ub<U, T>(genHalfBits<U>() - 1),
    avoid_ub<U, T>(genHalfBits<U>()),
    avoid_ub<U, T>(genHalfBits<U>() + 1),
    avoid_ub<U, T>(std::numeric_limits<U>::max() - 1),
    avoid_ub<U, T>(std::numeric_limits<U>::max()),
    avoid_ub<U, T>(std::numeric_limits<U>::max() - 0xff),
    avoid_ub<U, T>(std::numeric_limits<U>::max() - 0xff),
    avoid_ub<U, T>(std::numeric_limits<U>::max() - 0x55),
    avoid_ub<U, T>(-(std::numeric_limits<U>::min() + 1)),
    avoid_ub<U, T>(-std::numeric_limits<U>::max()),
    avoid_ub<U, T>(std::numeric_limits<U>::max() / std::pow(2., sizeof(T) * 6 - 1)),
    avoid_ub2<U, T>(-std::numeric_limits<U>::max() / std::pow(2., sizeof(T) * 6 - 1)),
    avoid_ub<U, T>(std::numeric_limits<U>::max() / std::pow(2., sizeof(T) * 4 - 1)),
    avoid_ub2<U, T>(-std::numeric_limits<U>::max() / std::pow(2., sizeof(T) * 4 - 1)),
    avoid_ub<U, T>(std::numeric_limits<U>::max() / std::pow(2., sizeof(T) * 2 - 1)),
    avoid_ub2<U, T>(-std::numeric_limits<U>::max() / std::pow(2., sizeof(T) * 2 - 1)),
    avoid_ub<U, T>(std::numeric_limits<T>::max() - 1),
    avoid_ub<U, T>(std::numeric_limits<T>::max() * 0.75),
}};

template <class T, class U> struct cvt_inputs {
    static constexpr size_t size() { return cvt_input_data<U, T>.size(); }
    U operator[](size_t i) const { return cvt_input_data<U, T>[i]; }
};
