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
#include "./effects.h"
#include "./timeline.h"
#include "./leds.h"
#include "./color.h"
#include "./fastmath.h"

#include <random>
#include <array>
#include <limits>
#include <math.h>

static constexpr vector::float4 gradient_rainbow_data[] = {
    color::srgb8_stop({0xff,0x00,0x00}, 0.00f),
    color::srgb8_stop({0xff,0xff,0x00}, 0.16f),
    color::srgb8_stop({0x00,0xff,0x00}, 0.33f),
    color::srgb8_stop({0x00,0xff,0xff}, 0.50f),
    color::srgb8_stop({0x00,0x00,0xff}, 0.66f),
    color::srgb8_stop({0xff,0x00,0xff}, 0.83f),
    color::srgb8_stop({0xff,0x00,0x00}, 1.00f)};
static const color::gradient gradient_rainbow(gradient_rainbow_data,7);

Effects &Effects::instance() {
    static Effects effects;
    if (!effects.initialized) {
        effects.initialized = true;
        effects.init();
    }
    return effects;
}

void Effects::init() {

    random.set_seed(0x1ED51ED5);

    static Timeline::Span mainEffect;

    if (!Timeline::instance().Scheduled(mainEffect)) {
        mainEffect.type = Timeline::Span::Effect;
        mainEffect.time = Timeline::SystemTime();
        mainEffect.duration = std::numeric_limits<double>::infinity();
        mainEffect.calcFunc = [this](Timeline::Span &span, Timeline::Span &) {
            double now = Timeline::SystemTime() - span.time;

            const double speed = 1.0;
            float walk = (1.0f - static_cast<float>(frac(now * speed)));

            auto calcRingColor = [=](const vector::float4 &pos) {
                return gradient_rainbow.repeat(pos.w + walk);
            };

            auto calcBirdColor = [=](const vector::float4 &pos) {
                return gradient_rainbow.repeat(pos.x + walk);
            };

            auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
            	size_t portLedCount = Leds::instance().portLedCount(port);
                for (size_t d = 0; d < portLedCount ; d++) {
                	Leds::instance().setCol(port, d, calc(Leds::instance().map(port,d)));
                }
            };

            iteratePort(0, calcBirdColor);
            iteratePort(1, calcBirdColor);
            iteratePort(2, calcBirdColor);
            iteratePort(3, calcBirdColor);
            iteratePort(4, calcBirdColor);
            iteratePort(5, calcRingColor);
        };
    }

    mainEffect.commitFunc = [this](Timeline::Span &) {
        Leds::instance().convert();
    };

    Timeline::instance().Add(mainEffect);
}

