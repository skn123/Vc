/*  This file is part of the Vc library. {{{

    Copyright (C) 2013 Matthias Kretz <kretz@kde.org>

    Vc is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    Vc is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Vc.  If not, see <http://www.gnu.org/licenses/>.

}}}*/

#ifndef VC_COMMON_SIMD_ARRAY_DATA_H
#define VC_COMMON_SIMD_ARRAY_DATA_H

#include "macros.h"

Vc_NAMESPACE_BEGIN(Common)

template<typename V, std::size_t N> struct ArrayData;
template<typename M, std::size_t N> struct MaskData;

template<typename V> struct ArrayData<V, 1>
{
    typedef typename V::EntryType value_type;

    V d;

    V *begin() { return &d; }
    const V *cbegin() const { return &d; }

    V *end() { return &d + 1; }
    const V *cend() { return &d + 1; }

    ArrayData() = default;
    Vc_ALWAYS_INLINE ArrayData(const V &x) : d(x) {}
    Vc_ALWAYS_INLINE ArrayData(const value_type *x) : d(x) {}
    template<typename Flags> Vc_ALWAYS_INLINE ArrayData(const value_type *x, Flags flags)
        : d(x, flags) {}
    template<typename U, typename Flags> Vc_ALWAYS_INLINE ArrayData(const U *x, Flags flags)
        : d(x, flags) {}

    Vc_ALWAYS_INLINE ArrayData(VectorSpecialInitializerIndexesFromZero::IEnum x)
        : d(x)
    {
    }
    Vc_ALWAYS_INLINE ArrayData(VectorSpecialInitializerIndexesFromZero::IEnum x, size_t offset)
        : d(x)
    {
        d += offset;
    }

    template<typename U, typename Flags>
    Vc_ALWAYS_INLINE void load(const U *x, Flags f) {
        d.load(x, f);
    }

    template<typename F, typename... Args>
    inline void call(F function, Args... args) {
        (d.*function)(args...);
    }
};
template<typename V, std::size_t N> struct ArrayData
{
    static_assert(N != 0, "error N must be nonzero!");
    typedef typename V::EntryType value_type;

    V d;
    ArrayData<V, N - 1> next;

    V *begin() { return &d; }
    const V *cbegin() const { return &d; }

    V *end() { return next.end(); }
    const V *cend() { return next.cend(); }

    ArrayData() = default;
    Vc_ALWAYS_INLINE ArrayData(const V &x) : d(x), next(x) {}
    Vc_ALWAYS_INLINE ArrayData(const value_type *x) : d(x), next(x + V::Size) {}
    template<typename Flags> Vc_ALWAYS_INLINE ArrayData(const value_type *x, Flags flags)
        : d(x, flags), next(x + V::Size, flags) {}
    template<typename U, typename Flags> Vc_ALWAYS_INLINE ArrayData(const U *x, Flags flags)
        : d(x, flags), next(x + V::Size, flags) {}

    Vc_ALWAYS_INLINE ArrayData(VectorSpecialInitializerIndexesFromZero::IEnum x)
        : d(x), next(x, V::Size)
    {
    }
    Vc_ALWAYS_INLINE ArrayData(VectorSpecialInitializerIndexesFromZero::IEnum x, size_t offset)
        : d(x), next(x, offset + V::Size)
    {
        d += offset;
    }

    template<typename U, typename Flags>
    Vc_ALWAYS_INLINE void load(const U *x, Flags f) {
        d.load(x, f);
        next.load(x + V::Size, f);
    }

    template<typename F, typename... Args>
    inline void call(F function, Args... args) {
        (d.*function)(args...);
        next.call(function, args...);
    }
};

template<typename M, std::size_t N> struct MaskData;
template<typename M> struct MaskData<M, 1>
{
    M *begin() { return &d; }
    const M *cbegin() const { return &d; }

    M *end() { return &d + 1; }
    const M *cend() { return &d + 1; }

    MaskData() = default;
    Vc_ALWAYS_INLINE MaskData(const M &x) : d(x) {}

    Vc_ALWAYS_INLINE Vc_PURE bool isFull() const { return d.isFull(); }
    Vc_ALWAYS_INLINE Vc_PURE bool isEmpty() const { return d.isEmpty(); }

    template<typename V, typename F>
    Vc_ALWAYS_INLINE void assign(const ArrayData<V, 1> &lhs, const ArrayData<V, 1> &rhs, F function) {
        d = (lhs.d.*function)(rhs.d);
    }

//private:
    M d;
};
template<typename M, std::size_t N> struct MaskData
{
    static_assert(N != 0, "error N must be nonzero!");

    M *begin() { return &d; }
    const M *cbegin() const { return &d; }

    M *end() { return next.end(); }
    const M *cend() { return next.cend(); }

    MaskData() = default;
    Vc_ALWAYS_INLINE MaskData(const M &x) : d(x), next(x) {}

    Vc_ALWAYS_INLINE Vc_PURE bool isFull() const { return d.isFull() && next.isFull(); }
    Vc_ALWAYS_INLINE Vc_PURE bool isEmpty() const { return d.isEmpty() && next.isEmpty(); }

    template<typename V, typename F>
    Vc_ALWAYS_INLINE void assign(const ArrayData<V, N> &lhs, const ArrayData<V, N> &rhs, F function) {
        d = (lhs.d.*function)(rhs.d);
        next.assign(lhs.next, rhs.next, function);
    }

//private:
    M d;
    MaskData<M, N - 1> next;
};

Vc_NAMESPACE_END

#include "undomacros.h"

#endif // VC_COMMON_SIMD_ARRAY_DATA_H
