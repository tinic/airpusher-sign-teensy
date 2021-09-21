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

#include "fsl_debug_console.h"

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

static constexpr vector::float4 red_glow_data[] = {
    color::srgb8_stop({0xff,0x00,0x00}, 0.00f),
    color::srgb8_stop({0xff,0x1f,0x00}, 0.16f),
    color::srgb8_stop({0xff,0x00,0x00}, 0.33f),
    color::srgb8_stop({0xff,0x00,0x1f}, 0.50f),
    color::srgb8_stop({0xff,0x00,0x00}, 0.66f),
    color::srgb8_stop({0xff,0x0f,0x0f}, 0.83f),
    color::srgb8_stop({0xff,0x00,0x00}, 1.00f)};
static const color::gradient red_glow(red_glow_data,7);

static constexpr vector::float4 rainbow_bright_data[] = {
    color::srgb8_stop(0xff0000, 0.00f),
    color::srgb8_stop(0xffbd96, 0.10f),
    color::srgb8_stop(0xffff00, 0.17f),
    color::srgb8_stop(0xc3ffa9, 0.25f),
    color::srgb8_stop(0x00ff00, 0.33f),
    color::srgb8_stop(0xd1ffbf, 0.38f),
    color::srgb8_stop(0xaffff3, 0.44f),
    color::srgb8_stop(0x29fefe, 0.50f),
    color::srgb8_stop(0x637eff, 0.59f),
    color::srgb8_stop(0x0000ff, 0.67f),
    color::srgb8_stop(0x9c3fff, 0.75f),
    color::srgb8_stop(0xff00ff, 0.83f),
    color::srgb8_stop(0xffc2b0, 0.92f),
    color::srgb8_stop(0xff0000, 1.00f)};
static const color::gradient rainbow_bright_gradient(rainbow_bright_data,14);

static constexpr vector::float4 rainy_data[] = {
    color::srgb8_stop(0x000000, 0.00f),
    color::srgb8_stop(0x413a40, 0.20f),
    color::srgb8_stop(0x65718a, 0.40f),
    color::srgb8_stop(0x6985b9, 0.53f),
    color::srgb8_stop(0xffffff, 1.00f)};
static const color::gradient rainy_gradient(rainy_data,5);

static constexpr vector::float4 autumn_data[] = {
    color::srgb8_stop(0x000000, 0.00f),
    color::srgb8_stop(0x351e10, 0.13f),
    color::srgb8_stop(0x58321a, 0.25f),
    color::srgb8_stop(0x60201e, 0.41f),
    color::srgb8_stop(0x651420, 0.56f),
    color::srgb8_stop(0x7b5a54, 0.70f),
    color::srgb8_stop(0x9abf9e, 0.83f),
    color::srgb8_stop(0xffffff, 1.00f)};
static const color::gradient autumn_gradient(autumn_data,8);

static constexpr vector::float4 winter_data[] = {
    color::srgb8_stop(0xa3eed6, 0.00f),
    color::srgb8_stop(0xdcbcd4, 0.21f),
    color::srgb8_stop(0xff96d0, 0.39f),
    color::srgb8_stop(0xcb81d6, 0.65f),
    color::srgb8_stop(0x4b51f5, 1.00f)};
static const color::gradient winter_gradient(winter_data,5);

static constexpr vector::float4 happy_data[] = {
    color::srgb8_stop(0x22c1c3, 0.00f),
    color::srgb8_stop(0x4387c0, 0.33f),
    color::srgb8_stop(0xbb6161, 0.66f),
    color::srgb8_stop(0xfdbb2d, 1.00f)};
static const color::gradient happy_gradient(happy_data,4);

static constexpr vector::float4 evening_data[] = {
    color::srgb8_stop(0x000000, 0.00f),
    color::srgb8_stop(0x4387c0, 0.80f),
    color::srgb8_stop(0xbb6161, 0.90f),
    color::srgb8_stop(0xff9500, 0.95f),
    color::srgb8_stop(0xffffff, 1.00f)};
static const color::gradient evening_gradient(evening_data,5);

static constexpr vector::float4 desertdream_data[] = {
    color::srgb8_stop(0x4d5951, 0.00f),
    color::srgb8_stop(0x372a25, 0.19f),
    color::srgb8_stop(0x863c25, 0.41f),
    color::srgb8_stop(0xa15123, 0.63f),
    color::srgb8_stop(0xd6aa68, 0.84f),
    color::srgb8_stop(0xf7d6b4, 1.00f)};
static const color::gradient desertdream_gradient(desertdream_data,6);

static constexpr vector::float4 inthejungle_data[] = {
    color::srgb8_stop(0x135e46, 0.00f),
    color::srgb8_stop(0x478966, 0.20f),
    color::srgb8_stop(0x73a788, 0.40f),
    color::srgb8_stop(0xe3c6ad, 0.70f),
    color::srgb8_stop(0xd09d7b, 0.90f),
    color::srgb8_stop(0xb67b65, 1.00f)};
static const color::gradient inthejungle_gradient(inthejungle_data,6);

static constexpr vector::float4 darklight_data[] = {
    color::srgb8_stop(0x000000, 0.00f),
    color::srgb8_stop(0x135e46, 0.50f),
    color::srgb8_stop(0x2ea61b, 0.65f),
    color::srgb8_stop(0x478966, 0.70f),
    color::srgb8_stop(0x000000, 1.00f)};
static const color::gradient darklight_gradient(darklight_data,5);

static constexpr vector::float4 glow_wipe_data[] = {
    color::srgb8_stop({0x00,0x00,0x00}, 0.00f),
    color::srgb8_stop({0x00,0x00,0x00}, 0.45f),
    color::srgb8_stop({0xff,0xff,0xff}, 0.50f),
    color::srgb8_stop({0x00,0x00,0x00}, 0.55f),
    color::srgb8_stop({0x00,0x00,0x00}, 1.00f)};
static const color::gradient glow_wipe_gradient(glow_wipe_data,5);

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

void Effects::basic(const Timeline::Span &span) {

    double now = Timeline::SystemTime() - span.time;

    const double speed = 0.25;
    float walk = (1.0f - static_cast<float>(frac(now * speed)));

    auto calcRingColor = [=](const vector::float4 &pos) {
        return rainbow_bright_gradient.reflect(pos.w + walk * 4);
    };

    auto calcBirdColor = [=](const vector::float4 &pos) {
        return rainbow_bright_gradient.reflect(pos.x + walk * 4);
    };

    auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
        size_t portLedCount = Leds::instance().portLedCount(port);
        Leds &leds(Leds::instance());
        for (size_t d = 0; d < portLedCount ; d++) {
            //Leds::instance().setCol(port, d, segment_color[Leds::instance().seg(port,d)]);
            leds.setCol(port, d, calc(Leds::instance().map(port,d)));
        }
    };

    iteratePort(0, calcBirdColor);
    iteratePort(1, calcBirdColor);
    iteratePort(2, calcBirdColor);
    iteratePort(3, calcBirdColor);
    iteratePort(4, calcBirdColor);
    iteratePort(5, calcRingColor);
}

void Effects::redglow(const Timeline::Span &span) {

    double now = Timeline::SystemTime() - span.time;

    const double speed = 0.25;
    float walk = (1.0f - static_cast<float>(frac(now * speed)));

    auto calcRingColor = [=](const vector::float4 &pos) {
        return red_glow.reflect(pos.w + walk);
    };

    auto calcBirdColor = [=](const vector::float4 &pos) {
        return red_glow.reflect(pos.x + walk);
    };

    auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
        size_t portLedCount = Leds::instance().portLedCount(port);
        Leds &leds(Leds::instance());
        for (size_t d = 0; d < portLedCount ; d++) {
            leds.setCol(port, d, calc(Leds::instance().map(port,d)));
        }
    };

    iteratePort(0, calcBirdColor);
    iteratePort(1, calcBirdColor);
    iteratePort(2, calcBirdColor);
    iteratePort(3, calcBirdColor);
    iteratePort(4, calcBirdColor);

    iteratePort(5, calcRingColor);
}

void Effects::spring(const Timeline::Span &span) {

    double now = Timeline::SystemTime() - span.time;

    auto calcRingColor = [=](const vector::float4 &pos) {
        return gradient_rainbow.reflect(pos.w);
    };

    auto calcBirdColor = [=](const vector::float4 &pos) {
        float x = sinf((pos.x + 1.0f) * 0.25f + float(now) * 0.050f);
        float y = cosf((pos.y + 1.0f) * 0.25f + float(now) * 0.055f);
        float l = 1.0f - pos.xy00().len() + 0.5f;
        return rainbow_bright_gradient.reflect(x * y) * l;
    };

    auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
        size_t portLedCount = Leds::instance().portLedCount(port);
        Leds &leds(Leds::instance());
        for (size_t d = 0; d < portLedCount ; d++) {
            leds.setCol(port, d, calc(Leds::instance().map(port,d)));
        }
    };

    iteratePort(0, calcBirdColor);
    iteratePort(1, calcBirdColor);
    iteratePort(2, calcBirdColor);
    iteratePort(3, calcBirdColor);
    iteratePort(4, calcBirdColor);

    iteratePort(5, calcRingColor);
}

void Effects::summer(const Timeline::Span &span) {

    double now = Timeline::SystemTime() - span.time;

    auto calcRingColor = [=](const vector::float4 &pos) {
        return gradient_rainbow.reflect(pos.w);
    };

    auto calcBirdColor = [=](const vector::float4 &pos) {
        float x0 = sinf((pos.x + 1.0f) * 0.5f + float(now) * 0.050f);
        float y0 = cosf((pos.y + 1.0f) * 0.5f + float(now) * 0.055f);
        float x1 = sinf((pos.x + 1.0f) * 10.0f + float(now) * 0.50f);
        float y1 = cosf((pos.y + 1.0f) * 10.0f + float(now) * 0.55f);
        return (rainbow_bright_gradient.reflect(x0 * y0) + vector::float4::one()) * (x1 * y1);
    };

    auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
        size_t portLedCount = Leds::instance().portLedCount(port);
        Leds &leds(Leds::instance());
        for (size_t d = 0; d < portLedCount ; d++) {
            leds.setCol(port, d, calc(Leds::instance().map(port,d)));
        }
    };

    iteratePort(0, calcBirdColor);
    iteratePort(1, calcBirdColor);
    iteratePort(2, calcBirdColor);
    iteratePort(3, calcBirdColor);
    iteratePort(4, calcBirdColor);

    iteratePort(5, calcRingColor);
}

void Effects::autumn(const Timeline::Span &span) {

    double now = Timeline::SystemTime() - span.time;

    auto calcRingColor = [=](const vector::float4 &pos) {
        return gradient_rainbow.reflect(pos.w);
    };

    auto calcBirdColor = [=](const vector::float4 &pos) {
        float x0 = sinf((pos.x + 1.0f) * 0.5f + float(now) * 0.050f);
        float y0 = cosf((pos.y + 1.0f) * 0.5f + float(now) * 0.055f);
        float x1 = sinf((pos.x + 1.0f) * 10.0f + float(now) * 0.50f);
        float y1 = cosf((pos.y + 1.0f) * 10.0f + float(now) * 0.55f);
        return (rainy_gradient.clamp(x1 * y1) + autumn_gradient.reflect(x0 * y0)) * vector::float4::half();
    };

    auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
        size_t portLedCount = Leds::instance().portLedCount(port);
        Leds &leds(Leds::instance());
        for (size_t d = 0; d < portLedCount ; d++) {
            leds.setCol(port, d, calc(Leds::instance().map(port,d)));
        }
    };

    iteratePort(0, calcBirdColor);
    iteratePort(1, calcBirdColor);
    iteratePort(2, calcBirdColor);
    iteratePort(3, calcBirdColor);
    iteratePort(4, calcBirdColor);

    iteratePort(5, calcRingColor);
}

void Effects::winter(const Timeline::Span &span) {

    double now = Timeline::SystemTime() - span.time;

    auto calcRingColor = [=](const vector::float4 &pos) {
        return gradient_rainbow.reflect(pos.w);
    };

    auto calcBirdColor = [=](const vector::float4 &pos) {
        double x0 = sin((double(pos.x) + 1.0) * 0.5 + now * 0.050);
        double y0 = cos((double(pos.y) + 1.0) * 0.5 + now * 0.055);
        double x1 = sin((double(pos.x) + 1.0) * 0.25 + now * 0.050);
        double y1 = cos((double(pos.y) + 1.0) * 0.25 + now * 0.055);
        double l = 1.0 - double(pos.xy00().len()) + 0.5;
        return winter_gradient.reflect(float(x1 * y1)) * rainy_gradient.reflect(float(x0 * y0)) * float(l);
    };

    auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
        size_t portLedCount = Leds::instance().portLedCount(port);
        Leds &leds(Leds::instance());
        for (size_t d = 0; d < portLedCount ; d++) {
            leds.setCol(port, d, calc(Leds::instance().map(port,d)));
        }
    };

    iteratePort(0, calcBirdColor);
    iteratePort(1, calcBirdColor);
    iteratePort(2, calcBirdColor);
    iteratePort(3, calcBirdColor);
    iteratePort(4, calcBirdColor);

    iteratePort(5, calcRingColor);
}


void Effects::afterrain(const Timeline::Span &span) {

    double now = Timeline::SystemTime() - span.time;

    auto calcRingColor = [=](const vector::float4 &pos) {
        return gradient_rainbow.reflect(pos.w);
    };

    auto calcBirdColor = [=](const vector::float4 &pos) {
        double b = (sin(double(pos.x) * 4.0 + now * 0.20) + cos(double(pos.y) * 4.0 + now * 0.20)) * 0.25;
        vector::float4 p = (pos.rotate2d(float(now * 0.20)) + vector::float4(float(now * 0.20), 0.0f, 0.0f, 0.0f)) * 0.05f;
        return rainbow_bright_gradient.repeat(p.x) + vector::float4(float(b),float(b),float(b),float(b));
    };

    auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
        size_t portLedCount = Leds::instance().portLedCount(port);
        Leds &leds(Leds::instance());
        for (size_t d = 0; d < portLedCount ; d++) {
            leds.setCol(port, d, calc(Leds::instance().map(port,d)));
        }
    };

    iteratePort(0, calcBirdColor);
    iteratePort(1, calcBirdColor);
    iteratePort(2, calcBirdColor);
    iteratePort(3, calcBirdColor);
    iteratePort(4, calcBirdColor);

    iteratePort(5, calcRingColor);
}

void Effects::sunsetsunrise(const Timeline::Span &span) {

    double now = Timeline::SystemTime() - span.time;

    auto calcRingColor = [=](const vector::float4 &pos) {
        return gradient_rainbow.reflect(pos.w);
    };

    auto calcBirdColor = [=](const vector::float4 &pos) {
        double a = std::max(0.0, cos(double(pos.x) + sin(now * 0.10)) + sin(double(pos.y) + cos(now * 0.10) ) - 1.0);
        vector::float4 p = (pos.rotate2d(float(now * 0.30)) + vector::float4(float(now * 0.30), 0.0f, 0.0f, 0.0f) ) * 0.05f;
        double l = 1.0 - double(pos.xy00().len()) + 0.5;
        vector::float4 c0 = happy_gradient.reflect(p.x) * float(l);
        vector::float4 c1 = evening_gradient.clamp(float(a));
        return vector::float4::lerp(c0, c1, float(a));
    };

    auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
        size_t portLedCount = Leds::instance().portLedCount(port);
        Leds &leds(Leds::instance());
        for (size_t d = 0; d < portLedCount ; d++) {
            leds.setCol(port, d, calc(Leds::instance().map(port,d)));
        }
    };

    iteratePort(0, calcBirdColor);
    iteratePort(1, calcBirdColor);
    iteratePort(2, calcBirdColor);
    iteratePort(3, calcBirdColor);
    iteratePort(4, calcBirdColor);

    iteratePort(5, calcRingColor);
}

void Effects::desertdream(const Timeline::Span &span) {

    double now = Timeline::SystemTime() - span.time;

    auto calcRingColor = [=](const vector::float4 &pos) {
        return gradient_rainbow.reflect(pos.w);
    };

    auto calcBirdColor = [=](const vector::float4 &pos) {
        double x0 = sin((double(pos.x) + 1.0) * 0.5 + now * 0.050);
        double y0 = cos((double(pos.y )+ 1.0) * 0.5 + now * 0.055);
        double x1 = sin((double(pos.x) + 1.0) * 0.25 + now * 0.050);
        double y1 = cos((double(pos.y) + 1.0) * 0.25 + now * 0.055);
        double l = 1.0 - double(pos.xy00().len()) + 0.5;
        return (desertdream_gradient.reflect(float(x1 * y1)) * float(l)) + desertdream_gradient.reflect(float(x0 * y0));
    };

    auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
        size_t portLedCount = Leds::instance().portLedCount(port);
        Leds &leds(Leds::instance());
        for (size_t d = 0; d < portLedCount ; d++) {
            leds.setCol(port, d, calc(Leds::instance().map(port,d)));
        }
    };

    iteratePort(0, calcBirdColor);
    iteratePort(1, calcBirdColor);
    iteratePort(2, calcBirdColor);
    iteratePort(3, calcBirdColor);
    iteratePort(4, calcBirdColor);

    iteratePort(5, calcRingColor);
}

void Effects::inthejungle(const Timeline::Span &span) {

    double now = Timeline::SystemTime() - span.time;

    auto calcRingColor = [=](const vector::float4 &pos) {
        return gradient_rainbow.reflect(pos.w);
    };

    auto calcBirdColor = [=](const vector::float4 &pos) {
        double a = std::max(0.0, cos(double(pos.x) + sin(now * 0.10)) + sin(double(pos.y) + cos(now * 0.10))-1.0);
        vector::float4 p = (pos.rotate2d(float(now * 0.30)) + vector::float4(float(now * 0.30), 0.0f, 0.0f, 0.0f) ) * 0.05f;
        double l = 1.0 - double(pos.xy00().len()) + 0.5;
        vector::float4 c0 = inthejungle_gradient.reflect(p.x) * float(l);
        vector::float4 c1 = darklight_gradient.clamp(float(a));
        return vector::float4::lerp(c0, c1, float(a));
    };

    auto iteratePort = [](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
        size_t portLedCount = Leds::instance().portLedCount(port);
        Leds &leds(Leds::instance());
        for (size_t d = 0; d < portLedCount ; d++) {
            leds.setCol(port, d, calc(Leds::instance().map(port,d)));
        }
    };

    iteratePort(0, calcBirdColor);
    iteratePort(1, calcBirdColor);
    iteratePort(2, calcBirdColor);
    iteratePort(3, calcBirdColor);
    iteratePort(4, calcBirdColor);

    iteratePort(5, calcRingColor);
}

void Effects::init() {

    random.set_seed(0x1ED51ED5);

    static Timeline::Span mainEffect;

    static uint32_t current_effect = 0;
    static uint32_t previous_effect = 0;
    static double switch_time = 0;

    if (!Timeline::instance().Scheduled(mainEffect)) {

        mainEffect.type = Timeline::Span::Effect;
        mainEffect.time = Timeline::SystemTime();
        mainEffect.duration = std::numeric_limits<double>::infinity();
        mainEffect.calcFunc = [this](Timeline::Span &span, Timeline::Span &) {

            Leds &leds(Leds::instance());

            if ( current_effect != scheduled_effect ) {
                previous_effect = current_effect;
                current_effect = scheduled_effect;
                switch_time = Timeline::SystemTime();
            }

            auto calc_effect = [this, span] (uint32_t effect) {
                switch (effect) {
                    case 0:
                        basic(span);
                    break;
                    case 1:
                        redglow(span);
                    break;
                    case 2:
                        spring(span);
                    break;
                    case 3:
                        summer(span);
                    break;
                    case 4:
                        autumn(span);
                    break;
                    case 5:
                        winter(span);
                    break;
                    case 6:
                        afterrain(span);
                    break;
                    case 7:
                        sunsetsunrise(span);
                    break;
                    case 8:
                        desertdream(span);
                    break;
                    case 9:
                        inthejungle(span);
                    break;
                }
            };

            double blend_duration = 0.5;
            double now = Timeline::SystemTime();
            
            if ((now - switch_time) < blend_duration) {
                calc_effect(previous_effect);

                auto colPrev = leds.get();

                calc_effect(current_effect);

                auto colNext = leds.get();

                float blend = static_cast<float>(now - switch_time) * (1.0f / static_cast<float>(blend_duration));

                for (size_t c = 0; c < colNext.size(); c++) {
                    for (size_t d = 0; d < colNext[c].size(); d++) {
                        colNext[c][d] = vector::float4::lerp(colPrev[c][d], colNext[c][d], blend);
                    }
                }

                leds.set(colNext);

            } else {
                calc_effect(current_effect);
            }
        };
    }

    mainEffect.commitFunc = [this](Timeline::Span &) {
        Leds::instance().convert();
    };

    Timeline::instance().Add(mainEffect);

    static Timeline::Span effectSwitcher;
    if (!Timeline::instance().Scheduled(effectSwitcher)) {
        effectSwitcher.type = Timeline::Span::Interval;
        effectSwitcher.interval = 120.0;
        effectSwitcher.time = Timeline::SystemTime() + 120.0; // Initial time

        effectSwitcher.startFunc = [](Timeline::Span &) {
            PRINTF("Switch effect at %f\r\n", Timeline::SystemTime());
            Effects::instance().NextEffect();
        };

        Timeline::instance().Add(effectSwitcher);
    }

    static Timeline::Span glowRepeater;
    if (!Timeline::instance().Scheduled(glowRepeater)) {

        glowRepeater.type = Timeline::Span::Interval;
        glowRepeater.interval = 60.0;
        glowRepeater.intervalFuzz = 60.00;
        glowRepeater.time = Timeline::SystemTime() + 60.0; // Initial time

        glowRepeater.startFunc = [this](Timeline::Span &) {

            static Timeline::Span glowEffect;

            static float orientation = 0.0f;

            orientation = random.get(0.0f, 2.0f * float(pi));

            if (!Timeline::instance().Scheduled(glowEffect)) {
                glowEffect.type = Timeline::Span::Effect;
                glowEffect.time = Timeline::SystemTime();
                glowEffect.duration = 1.0f;
                glowEffect.calcFunc = [this](Timeline::Span &span, Timeline::Span &below) {
                    below.Calc();
                    auto belowCol = Leds::instance().get();

                    double now = Timeline::SystemTime() - span.time;

                    float walk = float( ( now - glowEffect.time ) / glowEffect.duration ) - 0.5f;

                    auto calcBirdColor = [=](const vector::float4 &pos) {
                        return glow_wipe_gradient.clamp((pos + walk).rotate2d(orientation).x);
                    };

                    auto iteratePort = [belowCol](size_t port, std::function<const vector::float4(const vector::float4 &)> calc) {
                        Leds &leds(Leds::instance());
                        size_t portLedCount = leds.portLedCount(port);
                        for (size_t c = 0; c < portLedCount ; c++) {
                            leds.setCol(port, c, belowCol[port][c] + calc(leds.map(port, c)));
                        }
                    };

                    iteratePort(0, calcBirdColor);
                    iteratePort(1, calcBirdColor);
                    iteratePort(2, calcBirdColor);
                    iteratePort(3, calcBirdColor);
                    iteratePort(4, calcBirdColor);

                };
                glowEffect.commitFunc = [this](Timeline::Span &) {
                    Leds::instance().convert();
                };
                Timeline::instance().Add(glowEffect);
            }

            PRINTF("Glow at %f\r\n", Timeline::SystemTime());
        };

        Timeline::instance().Add(glowRepeater);

    }
}
