
/*
Copyright 2021 Tinic Uro

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef FASTMATH_H_
#define FASTMATH_H_

#include <stdint.h>
#include <math.h>

#include "./gcem/gcem.hpp"

__attribute__ ((hot, optimize("Os"), flatten))
static inline float fast_rcp(const float x ) {
    // No std::bitcast yet
    const union { float f; uint32_t i; } u = { x };
    const union { uint32_t i; float f; } v = { ( 0xbe6eb3beU - u.i ) >> 1 };
    return v.f * v.f;
}

__attribute__ ((hot, optimize("Os"), flatten))
static inline float fast_rsqrt(const float x) {
    const union { float f; uint32_t i; } u = { x };
    const union { uint32_t i; float f; } v = { 0x5f375a86U - ( u.i >> 1 ) };
    return v.f * (1.5f - ( 0.5f * v.f * v.f ));
}

__attribute__ ((hot, optimize("Os"), flatten))
static inline float fast_exp2(const float p) {
    const float offset = (p < 0) ? 1.0f : 0.0f;
    const float clipp = (p < -126) ? -126.0f : p;
    const int w = static_cast<int>(clipp);
    const float z = clipp - w + offset;
    // No std::bitcast yet
    const union { uint32_t i; float f; } v = {
        static_cast<uint32_t>((1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z))
    };
    return v.f;
}

__attribute__ ((hot, optimize("Os"), flatten))
static inline float fast_log2(const float x) {
    // No std::bitcast yet
    const union { float f; uint32_t i; } vx = { x };
    const union { uint32_t i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
    const float y = static_cast<float>(vx.i) * 1.1920928955078125e-7f;
    return y - 124.22551499f
             - 1.498030302f * mx.f
             - 1.72587999f / (0.3520887068f + mx.f);
}

__attribute__ ((hot, optimize("Os"), flatten))
static inline float fast_pow(const float x, const float p) {
    return fast_exp2(p * fast_log2(x));
}

__attribute__ ((hot, optimize("Os"), flatten))
static inline double frac(double v) { // same as fmod(v, 1.0)
    return v - std::trunc(v);
}

__attribute__ ((hot, optimize("Os"), flatten))
static inline float fracf(float v) { // same as fmodf(v, 1.0f)
    return v - std::truncf(v);
}

__attribute__ ((hot, optimize("Os"), flatten))
static constexpr float constexpr_pow(const float x, const float p) {
#ifdef GCEM_VERSION_MAJOR
	return gcem::pow(x, p);
#else // #ifdef GCEM_VERSION_MAJOR
    return ::exp2f(p * ::log2f(x));
#endif  // #ifdef GCEM_VERSION_MAJOR
}

#endif  // #ifndef FASTMATH_H_
