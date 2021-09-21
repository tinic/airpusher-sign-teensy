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

static constexpr vector::float4 glow_wipe_data[] = {
    color::srgb8_stop({0x00,0x00,0x00}, 0.00f),
    color::srgb8_stop({0x00,0x00,0x00}, 0.45f),
    color::srgb8_stop({0xff,0xff,0xff}, 0.50f),
    color::srgb8_stop({0x00,0x00,0x00}, 0.55f),
    color::srgb8_stop({0x00,0x00,0x00}, 1.00f)};
static const color::gradient glow_wipe(glow_wipe_data,5);

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
        return gradient_rainbow.reflect(pos.w + walk * 4);
    };

    auto calcBirdColor = [=](const vector::float4 &pos) {
        return gradient_rainbow.reflect(pos.x + walk * 4);
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
                        return glow_wipe.clamp((pos + walk).rotate2d(orientation).x);
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
