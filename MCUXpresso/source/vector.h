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
#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <cstdint>
#include <algorithm>
#include <cmath>
#include <cfloat>

namespace vector {

    struct float4 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 0.0f;

        constexpr float4() {
            this->x = 0.0f;
            this->y = 0.0f;
            this->z = 0.0f;
            this->w = 0.0f;
        }

        constexpr float4(const float4 &v, float a) {
            this->x = v.x;
            this->y = v.y;
            this->z = v.z;
            this->w = a;
        }

        constexpr float4(uint32_t c, float a) {
            this->x = ((c>>16)&0xFF)*(1.0f/255.0f);
            this->y = ((c>> 8)&0xFF)*(1.0f/255.0f);
            this->z = ((c>> 0)&0xFF)*(1.0f/255.0f);
            this->w = a;
        }

        constexpr float4(float v) {
            this->x = v;
            this->y = v;
            this->z = v;
            this->w = v;
        }

        constexpr float4(float _x, float _y, float _z, float _w = 0.0f) {
            this->x = _x;
            this->y = _y;
            this->z = _z;
            this->w = _w;
        }

        float4 &operator=(const float4& other) = default;

        constexpr float4 &operator+=(float v) {
            this->x += v;
            this->y += v;
            this->z += v;
            this->w += v;
            return *this;
        }

        constexpr float4 &operator-=(float v) {
            this->x -= v;
            this->y -= v;
            this->z -= v;
            this->w -= v;
            return *this;
        }

        constexpr float4 &operator*=(float v) {
            this->x *= v;
            this->y *= v;
            this->z *= v;
            this->w *= v;
            return *this;
        }

        constexpr float4 &operator/=(float v) {
            this->x /= v;
            this->y /= v;
            this->z /= v;
            this->w /= v;
            return *this;
        }

        float4 &operator%=(float v) {
            this->x = fmodf(this->x, v);
            this->y = fmodf(this->y, v);
            this->z = fmodf(this->z, v);
            this->w = fmodf(this->w, v);
            return *this;
        }

        constexpr float4 &operator+=(const float4 &b) {
            this->x += b.x;
            this->y += b.y;
            this->z += b.z;
            this->w += b.w;
            return *this;
        }

        constexpr float4 &operator-=(const float4 &b) {
            this->x += b.x;
            this->y += b.y;
            this->z += b.z;
            this->w += b.w;
            return *this;
        }

        constexpr float4 &operator*=(const float4 &b) {
            this->x += b.x;
            this->y += b.y;
            this->z += b.z;
            this->w += b.w;
            return *this;
        }

        constexpr float4 &operator/=(const float4 &b) {
            this->x += b.x;
            this->y += b.y;
            this->z += b.z;
            this->w += b.w;
            return *this;
        }

        float4 &operator%=(const float4 &b) {
            this->x = fmodf(this->x, b.x);
            this->y = fmodf(this->y, b.y);
            this->z = fmodf(this->z, b.z);
            this->w = fmodf(this->w, b.w);
            return *this;
        }

        constexpr float4 operator-() const {
            return float4(-this->x,
                          -this->y,
                          -this->z,
                          -this->w);
        }

        constexpr float4 operator+() const {
            return float4(+this->x,
                          +this->y,
                          +this->z,
                          +this->w);
        }

        constexpr float4 operator+(float v) const {
            return float4(this->x+v,
                          this->y+v,
                          this->z+v,
                          this->w+v);
        }

        constexpr float4 operator-(float v) const {
            return float4(this->x-v,
                          this->y-v,
                          this->z-v,
                          this->w-v);
        }

        constexpr float4 operator*(float v) const {
            return float4(this->x*v,
                          this->y*v,
                          this->z*v,
                          this->w*v);
        }

        constexpr float4 operator/(float v) const {
            return float4(this->x/v,
                          this->y/v,
                          this->z/v,
                          this->w/v);
        }

        float4 operator%(float v) const {
            return float4(fmodf(this->x,v),
                          fmodf(this->y,v),
                          fmodf(this->z,v),
                          fmodf(this->w,v));
        }

        constexpr float4 operator+(const float4 &b) const {
            return float4(this->x+b.x,
                          this->y+b.y,
                          this->z+b.z,
                          this->w+b.w);
        }

        constexpr float4 operator-(const float4 &b) const {
            return float4(this->x-b.x,
                          this->y-b.y,
                          this->z-b.z,
                          this->w-b.w);
        }

        constexpr float4 operator*(const float4 &b) const {
            return float4(this->x*b.x,
                          this->y*b.y,
                          this->z*b.z,
                          this->w*b.w);
        }

        constexpr float4 operator/(const float4 &b) const {
            return float4(this->x/b.x,
                          this->y/b.y,
                          this->z/b.z,
                          this->w/b.w);
        }

        float4 operator%(const float4 &b) const {
            return float4(fmodf(this->x,b.x),
                          fmodf(this->y,b.y),
                          fmodf(this->z,b.z),
                          fmodf(this->w,b.w));
        }

        float len() {
            return sqrtf(x*x + y*y + z*z);
        }

        static float len(const float4 &a) {
            return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
        }

        float dist(const float4 &b) {
            float xd = fabsf(this->x - b.x);
            float yd = fabsf(this->y - b.y);
            float zd = fabsf(this->z - b.z);
            return sqrtf(xd*xd + yd*yd + zd*zd);
        }

        static float dist(const float4 &a, const float4 &b) {
            float xd = fabsf(a.x - b.x);
            float yd = fabsf(a.y - b.y);
            float zd = fabsf(a.z - b.z);
            return sqrtf(xd*xd + yd*yd + zd*zd);
        }

        constexpr float4 min(const float4 &b) const {
            return float4(std::min(this->x, b.x),
                          std::min(this->y, b.y),
                          std::min(this->z, b.z),
                          std::min(this->w, b.w));
        }

        constexpr static float4 min(const float4 &a, const float4 &b) {
            return float4(std::min(a.x, b.x),
                          std::min(a.y, b.y),
                          std::min(a.z, b.z),
                          std::min(a.w, b.w));
        }

        constexpr float4 max(const float4 &b) const {
            return float4(std::max(this->x, b.x),
                          std::max(this->y, b.y),
                          std::max(this->z, b.z),
                          std::max(this->w, b.w));
        }

        constexpr static float4 max(const float4 &a, const float4 &b) {
            return float4(std::max(a.x, b.x),
                          std::max(a.y, b.y),
                          std::max(a.z, b.z),
                          std::max(a.w, b.w));
        }

        float4 pow(float v) {
            return float4(powf(this->x, v),
                          powf(this->y, v),
                          powf(this->z, v),
                          powf(this->w, v));
        }

        float4 abs() {
            return float4(fabsf(this->x),
                          fabsf(this->y),
                          fabsf(this->z),
                          fabsf(this->w));
        }

        static float4 abs(const float4 &a) {
            return float4(fabsf(a.x),
                          fabsf(a.y),
                          fabsf(a.z),
                          fabsf(a.w));
        }

        constexpr float4 xx00() const {
            return float4(this->x, this->x, 0.0, 0.0);
        }

        constexpr float4 yy00() const {
            return float4(this->y, this->y, 0.0, 0.0);
        }

        constexpr float4 zz00() const {
            return float4(this->z, this->z, 0.0, 0.0);
        }

        constexpr float4 xy00() const {
            return float4(this->x, this->y, 0.0, 0.0);
        }

        constexpr float4 yx00() const {
            return float4(this->y, this->x, 0.0, 0.0);
        }

        constexpr float4 xz00() const {
            return float4(this->x, this->z, 0.0, 0.0);
        }

        constexpr float4 zx00() const {
            return float4(this->z, this->x, 0.0, 0.0);
        }

        constexpr float4 yz00() const {
            return float4(this->y, this->z, 0.0, 0.0);
        }

        constexpr float4 zy00() const {
            return float4(this->z, this->y, 0.0, 0.0);
        }

        float4 sqrt() {
            return float4(sqrtf(this->x),
                          sqrtf(this->y),
                          sqrtf(this->z),
                          sqrtf(this->w));
        }

        static float4 sqrt(const float4 &a) {
            return float4(sqrtf(a.x),
                          sqrtf(a.y),
                          sqrtf(a.z),
                          sqrtf(a.w));
        }

        float4 rotate2d(float angle) {
            return float4(
                this->x * cosf(angle) - this->y * sinf(angle),
                this->y * cosf(angle) + this->x * sinf(angle),
                this->z,
                this->w);
        }

        float4 reflect() {
            return float4(
                reflect(this->x),
                reflect(this->y),
                reflect(this->z),
                reflect(this->w));
        }

        float4 rsqrt() {
            return float4((this->x != 0.0f) ? (1.0f/sqrtf(this->x)) : 0.0f,
                          (this->y != 0.0f) ? (1.0f/sqrtf(this->y)) : 0.0f,
                          (this->z != 0.0f) ? (1.0f/sqrtf(this->z)) : 0.0f,
                          (this->w != 0.0f) ? (1.0f/sqrtf(this->w)) : 0.0f);
        }

        static float4 rsqrt(const float4 &a) {
            return float4((a.x != 0.0f) ? (1.0f/sqrtf(a.x)) : 0.0f,
                          (a.y != 0.0f) ? (1.0f/sqrtf(a.y)) : 0.0f,
                          (a.z != 0.0f) ? (1.0f/sqrtf(a.z)) : 0.0f,
                          (a.w != 0.0f) ? (1.0f/sqrtf(a.w)) : 0.0f);
        }

        constexpr float4 rcp() const {
            return float4((this->x != 0.0f) ? (1.0f/this->x) : 0.0f,
                          (this->y != 0.0f) ? (1.0f/this->y) : 0.0f,
                          (this->z != 0.0f) ? (1.0f/this->z) : 0.0f,
                          (this->w != 0.0f) ? (1.0f/this->w) : 0.0f);
        }

        static float4 rcp(const float4 &a) {
            return float4((a.x != 0.0f) ? (1.0f/a.x) : 0.0f,
                          (a.y != 0.0f) ? (1.0f/a.y) : 0.0f,
                          (a.z != 0.0f) ? (1.0f/a.z) : 0.0f,
                          (a.w != 0.0f) ? (1.0f/a.w) : 0.0f);
        }

        constexpr float4 lerp(const float4 &b, float v) const {
            return float4(
              this->x * (1.0f - v) + b.x * v,
              this->y * (1.0f - v) + b.y * v,
              this->z * (1.0f - v) + b.z * v,
              this->w * (1.0f - v) + b.w * v);
        }

        constexpr static float4 lerp(const float4 &a, const float4 &b, float v) {
            return float4(
              a.x * (1.0f - v) + b.x * v,
              a.y * (1.0f - v) + b.y * v,
              a.z * (1.0f - v) + b.z * v,
              a.w * (1.0f - v) + b.w * v);
        }

        constexpr float4 clamp() const {
            return float4(
                std::min(std::max(this->x, 0.0f), 1.0f),
                std::min(std::max(this->y, 0.0f), 1.0f),
                std::min(std::max(this->z, 0.0f), 1.0f),
                std::min(std::max(this->w, 0.0f), 1.0f)
            );
        }

        constexpr static float4 zero() {
            return float4(0.0f, 0.0f, 0.0f, 0.0f);
        }

        constexpr static float4 one() {
            return float4(1.0f, 1.0f, 1.0f, 1.0f);
        }

        constexpr static float4 half() {
            return float4(0.5f, 0.5f, 0.5f, 0.5f);
        }

    private:

        float reflect(float i) {
            i = fabsf(i);
            if ((static_cast<int32_t>(i) & 1) == 0) {
                i = fmodf(i, 1.0f);
            } else {
                i = fmodf(i, 1.0f);
                i = 1.0f - i;
            }
            return i;
        }
    };
}

#endif  // #ifndef _VECTOR_H_

