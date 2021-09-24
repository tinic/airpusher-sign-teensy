/*
Copyright 2020 Tinic Uro

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
#ifndef EFFECTS_H_
#define EFFECTS_H_

#include "./timeline.h"

#include <stdint.h>

class Effects {
public:
    static Effects &instance();

    void NextEffect() {
        scheduled_effect ++;
        scheduled_effect %= effectsN;
    }

private:
    static constexpr float pi = 3.14159265359f;

    class pseudo_random {
    public:

        void set_seed(uint32_t seed) {
            uint32_t i;
            a = 0xf1ea5eed, b = c = d = seed;
            for (i=0; i<20; ++i) {
                (void)get();
            }
        }

        #define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
        uint32_t get() {
            uint32_t e = a - rot(b, 27);
            a = b ^ rot(c, 17);
            b = c + d;
            c = d + e;
            d = e + a;
            return d;
        }

        float get(float lower, float upper) {
            return static_cast<float>(static_cast<double>(get()) * (static_cast<double>(upper-lower)/static_cast<double>(1LL<<32)) ) + lower;
        }

        int32_t get(int32_t lower, int32_t upper) {
            return (static_cast<int32_t>(get()) % (upper-lower)) + lower;
        }

    private:
        uint32_t a;
        uint32_t b;
        uint32_t c;
        uint32_t d;

    } random;

    static constexpr size_t effectsN = 2;
    struct Effect {
        bool active = false;
        double time = 0.0;
        float orientation = 0.0f;
        std::function<void (Effect &effect, Timeline::Span &span)> startFunc;
        std::function<void (Effect &effect, Timeline::Span &span)> calcFunc;
        std::function<void (Effect &effect, Timeline::Span &span)> doneFunc;
    } effects[effectsN];

    void basic(const Timeline::Span &span);

    void spring(const Timeline::Span &span);
    void summer(const Timeline::Span &span);
    void autumn(const Timeline::Span &span);
    void winter(const Timeline::Span &span);
    void afterrain(const Timeline::Span &span);
    void sunsetsunrise(const Timeline::Span &span);
    void desertdream(const Timeline::Span &span);
    void inthejungle(const Timeline::Span &span);

    void redglow(const Timeline::Span &span);

    void init();
    bool initialized = false;

    uint32_t scheduled_effect = 0;
};

#endif /* EFFECTS_H_ */

