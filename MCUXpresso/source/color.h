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
#ifndef _COLOR_H_
#define _COLOR_H_

#include <cstdint>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <array>

#include "./vector.h"
#include "./fastmath.h"

namespace color {

    template<class T> struct rgba {

        T r;
        T g;
        T b;
        T a;

        consteval rgba() :
            r(0),
            g(0),
            b(0),
            a(0){
        }

        constexpr rgba(const rgba &from) :
            r(from.r),
            g(from.g),
            b(from.b),
            a(from.a) {
        }

        constexpr rgba(T _r, T _g, T _b) :
            r(_r),
            g(_g),
            b(_b),
            a(0) {
        }

        constexpr rgba(T _r, T _g, T _b, T _a) :
            r(_r),
            g(_g),
            b(_b),
            a(_a) {
        }

        constexpr rgba(const vector::float4 &from) {
            r = clamp_to_type(from.x);
            g = clamp_to_type(from.y);
            b = clamp_to_type(from.z);
            a = clamp_to_type(from.w);
        }

        rgba<T>& operator=(const rgba<T>& other) = default;

        rgba<T> fix_for_ws2816();

        uint16_t *write_rgba_bytes(uint16_t *dst) {
            *dst++ = r;
            *dst++ = g;
            *dst++ = b;
            *dst++ = a;
            return dst;
        }

        uint8_t *write_rgba_bytes(uint8_t *dst) {
            *dst++ = r;
            *dst++ = g;
            *dst++ = b;
            *dst++ = a;
            return dst;
        }

    private:
        T clamp_to_type(float v);
    };

    template<> inline rgba<uint16_t> rgba<uint16_t>::fix_for_ws2816() {
        return rgba<uint16_t>(  r < 384 ? ( ( r * 256 ) / 384 ) : r,
                                g < 384 ? ( ( g * 256 ) / 384 ) : g,
                                b < 384 ? ( ( b * 256 ) / 384 ) : b,
                                a < 384 ? ( ( a * 256 ) / 384 ) : a);
    }


    template<> __attribute__((always_inline)) inline float rgba<float>::clamp_to_type(float v) {
        return v;
    }

    template<> __attribute__((always_inline)) inline uint8_t rgba<uint8_t>::clamp_to_type(float v) {
        return __builtin_arm_usat(uint32_t(v * 255.f), 8);
    }

    template<> __attribute__((always_inline)) inline uint16_t rgba<uint16_t>::clamp_to_type(float v) {
        return __builtin_arm_usat(uint32_t(v * 65535.f), 16);
    }

    class gradient {
    public:
        consteval gradient(const vector::float4 stops[], const size_t n) {
            for (size_t c = 0; c < colors_n; c++) {
                float f = static_cast<float>(c) / static_cast<float>(colors_n - 1);
                vector::float4 a = stops[0];
                vector::float4 b = stops[1];
                if (n > 2) {
                    for (int32_t d = static_cast<int32_t>(n-2); d >= 0 ; d--) {
                        if ( f >= (stops[d].w) ) {
                            a = stops[d+0];
                            b = stops[d+1];
                            break;
                        }
                    }
                }
                f -= a.w;
                f /= b.w - a.w;
                colors[c] = a.lerp(b,f);
            }
        }

        vector::float4 repeat(float i) const;
        vector::float4 reflect(float i) const;
        vector::float4 clamp(float i) const;

    private:
        static constexpr size_t colors_n = 256;
        static constexpr float colors_mul = 255.0;
        static constexpr size_t colors_mask = 0xFF;

        vector::float4 colors[colors_n];
    };

    class convert {
    public:

        constexpr convert() : sRGB2lRGB() {
            for (size_t c = 0; c < 256; c++) {
                float v = float(c) / 255.0f;
                if (v > 0.04045f) {
                    sRGB2lRGB[c] = constexpr_pow( (v + 0.055f) / 1.055f, 2.4f);
                } else {
                    sRGB2lRGB[c] = v * ( 25.0f / 323.0f );
                };
            }
        }

        constexpr vector::float4 sRGB2CIELUV(const rgba<uint8_t> &in) const  {
            float r = sRGB2lRGB[in.r];
            float g = sRGB2lRGB[in.g];
            float b = sRGB2lRGB[in.b];

            float X = 0.4124564f * r + 0.3575761f * g + 0.1804375f * b;
            float Y = 0.2126729f * r + 0.7151522f * g + 0.0721750f * b;
            float Z = 0.0193339f * r + 0.1191920f * g + 0.9503041f * b;

            const float wu = 0.197839825f;
            const float wv = 0.468336303f;

            float l = ( Y <= 0.008856452f ) ? ( 9.03296296296f * Y) : ( 1.16f * constexpr_pow(Y, 1.0f / 3.0f) - 0.16f);
            float d = X + 15.f * Y + 3.0f * Z;
            float di = 1.0f / d;

            return vector::float4(l,
                (d > (1.0f / 65536.0f)) ? 13.0f * l * ( ( 4.0f * X * di ) - wu ) : 0.0f,
                (d > (1.0f / 65536.0f)) ? 13.0f * l * ( ( 9.0f * Y * di ) - wv ) : 0.0f);
        }

        vector::float4 CIELUV2sRGB(const vector::float4 &) const;

    private:
        float sRGB2lRGB[256];
    };

    constexpr vector::float4 lRGB2CIELUV(const vector::float4 &in) {
        float r = in.x;
        float g = in.y;
        float b = in.z;

        float X = 0.4124564f * r + 0.3575761f * g + 0.1804375f * b;
        float Y = 0.2126729f * r + 0.7151522f * g + 0.0721750f * b;
        float Z = 0.0193339f * r + 0.1191920f * g + 0.9503041f * b;

        const float wu = 0.197839825f;
        const float wv = 0.468336303f;

        float l = ( Y <= 0.008856452f ) ? ( 9.03296296296f * Y) : ( 1.16f * constexpr_pow(Y, 1.0f / 3.0f) - 0.16f);
        float d = X + 15.f * Y + 3.0f * Z;
        float di = 1.0f / d;

        return vector::float4(l,
            (d > (1.0f / 65536.0f)) ? 13.0f * l * ( ( 4.0f * X * di ) - wu ) : 0.0f,
            (d > (1.0f / 65536.0f)) ? 13.0f * l * ( ( 9.0f * Y * di ) - wv ) : 0.0f, in.w);
    }

    constexpr vector::float4 sRGB2CIELUV(const vector::float4 &in) {

        auto sRGB2lRGB = [](float v) {
            if (v > 0.04045f) {
                return constexpr_pow( (v + 0.055f) / 1.055f, 2.4f);
            } else {
                return v * ( 25.0f / 323.0f );
            };
        };

        float r = sRGB2lRGB(in.x);
        float g = sRGB2lRGB(in.y);
        float b = sRGB2lRGB(in.z);

        float X = 0.4124564f * r + 0.3575761f * g + 0.1804375f * b;
        float Y = 0.2126729f * r + 0.7151522f * g + 0.0721750f * b;
        float Z = 0.0193339f * r + 0.1191920f * g + 0.9503041f * b;

        const float wu = 0.197839825f;
        const float wv = 0.468336303f;

        float l = ( Y <= 0.008856452f ) ? ( 9.03296296296f * Y) : ( 1.16f * constexpr_pow(Y, 1.0f / 3.0f) - 0.16f);
        float d = X + 15.f * Y + 3.0f * Z;
        float di = 1.0f / d;

        return vector::float4(l,
            (d > (1.0f / 65536.0f)) ? 13.0f * l * ( ( 4.0f * X * di ) - wu ) : 0.0f,
            (d > (1.0f / 65536.0f)) ? 13.0f * l * ( ( 9.0f * Y * di ) - wv ) : 0.0f, in.w);
    }

    constexpr vector::float4 hsv(const vector::float4 &hsv) {
        float h = hsv.x;
        float s = hsv.y;
        float v = hsv.z;

        int32_t rd = static_cast<int32_t>( 6.0f * h );

        float f = h * 6.0f - rd;
        float p = v * (1.0f - s);
        float q = v * (1.0f - f * s);
        float t = v * (1.0f - (1.0f - f) * s);

        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;

        switch ( rd % 6 ) {
            default:
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            case 5: r = v; g = p; b = q; break;
        }

        return vector::float4(lRGB2CIELUV(vector::float4(r,g,b,hsv.w)));
    }

    constexpr vector::float4 srgb(const vector::float4 &color, float alpha = 1.0f) {
        return vector::float4(sRGB2CIELUV(color), alpha);
    }

    constexpr vector::float4 srgb8(const rgba<uint8_t> &color, float alpha = 1.0f) {
        return vector::float4(convert().sRGB2CIELUV(color), alpha);
    }

    constexpr vector::float4 srgb8_stop(const rgba<uint8_t> &color, float stop) {
        return vector::float4(convert().sRGB2CIELUV(color), stop);
    }
}

#endif  // #ifndef _COLOR_H_
