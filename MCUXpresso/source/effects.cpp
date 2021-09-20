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

static constexpr vector::float4 gradient_white_wipe_data[] = {
    color::srgb8_stop({0xff,0xff,0xff}, 0.00f),
    color::srgb8_stop({0x00,0x00,0x00}, 0.10f),
    color::srgb8_stop({0x00,0x00,0x00}, 1.00f)};
static const color::gradient gradient_wipe_white(gradient_white_wipe_data,3);

static constexpr vector::float4 segment_color [] = {
	color::srgb8({0xFF,0xFF,0xFF}),
	color::srgb8({0xFF,0x00,0x00}),
	color::srgb8({0x00,0xFF,0x00}),
	color::srgb8({0x00,0x00,0xFF}),
	color::srgb8({0xFF,0xFF,0x00}),
	color::srgb8({0x00,0xFF,0xFF}),
	color::srgb8({0xFF,0x00,0xFF}),
	color::srgb8({0x00,0x7F,0xFF}),
	color::srgb8({0xFF,0x7F,0x00}),
};

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

            const double speed = 0.25;
            float walk = (1.0f - static_cast<float>(frac(now * speed)));

            auto calcRingColor = [=](const vector::float4 &pos) {
                return gradient_wipe_white.reflect(pos.w + walk * 4);
            };

            auto calcBirdColor = [=](const vector::float4 &pos) {
                return gradient_wipe_white.reflect(pos.x + walk * 4);
            };

            auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
            	size_t portLedCount = Leds::instance().portLedCount(port);
                for (size_t d = 0; d < portLedCount ; d++) {
                	//Leds::instance().setCol(port, d, segment_color[Leds::instance().seg(port,d)]);
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

