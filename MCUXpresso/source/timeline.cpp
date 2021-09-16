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
#include "./timeline.h"

#include "clock_config.h"
#include "fsl_debug_console.h"

#include "MIMXRT1062.h"

#include <limits>
#include <array>

static uint32_t systemSeconds = 0;

extern "C" __attribute__((used)) void GPT1_IRQHandler(void) {
    GPT1->SR = GPT_SR_OF1(1);
    systemSeconds++;
    __DSB();
}

static bool effectReady = false;

extern "C" __attribute__((used)) void GPT2_IRQHandler(void) {
    GPT2->SR = GPT_SR_OF1(1);
    effectReady = true;
    __DSB();
}

float Quad::easeIn (float t,float b , float c, float d) {
    t /= d;
    return c*t*t+b;
}
float Quad::easeOut(float t,float b , float c, float d) {
    t /= d;
    return -c*t*(t-2.0f)+b;
}

float Quad::easeInOut(float t,float b , float c, float d) {
    t /= d/2.0f;
    if (t < 1.0f) {
        return ((c/2.0f)*t*t)+b;
    } else {
        t -= 1.0f;
        return -c/2.0f*(((t-2)*t)-1.0f)+b;
    }
}

float Cubic::easeIn (float t, float b, float c, float d) {
    t /= d;
    return c*t*t*t+b;
}

float Cubic::easeOut(float t, float b, float c, float d) {
    t = t/d-1.0f;
    return c*(t*t*t+1.0f)+b;
}

float Cubic::easeInOut(float t, float b, float c, float d) {
    t /= d/2.0f;
    if (t < 1.0f) {
        return c/2.0f*t*t*t+b;
    } else {
        t -= 2.0f;
        return c/2.0f*(t*t*t+2.0f)+b;
    }
}

Timeline &Timeline::instance() {
    static Timeline timeline;
    if (!timeline.initialized) {
        timeline.initialized = true;
        timeline.init();
    }
    return timeline;
}

bool Timeline::Scheduled(Timeline::Span &span) {
    for (Span *i = head; i ; i = i->next) {
        if ( i == &span ) {
            return true;
        }
    }
    return false;
}

void Timeline::Add(Timeline::Span &span) {
    for (Span *i = head; i ; i = i->next) {
        if ( i == &span ) {
            return;
        }
    }

    span.next = head;
    head = &span;
}

void Timeline::Remove(Timeline::Span &span) {
    Span *p = 0;
    for (Span *i = head; i ; i = i->next) {
        if ( i == &span ) {
            if (p) {
                p->next = i->next;
            } else {
                head = i->next;
            }
            i->next = 0;
            i->Done();
            return;
        }
        p = i;
    }
}

void Timeline::Process(Span::Type type) {
    static std::array<Span *, 64> collected;
    size_t collected_num = 0;
    double time = SystemTime();
    Span *p = 0;
    for (Span *i = head; i ; i = i->next) {
        if (i->type == type) {
            if ((i->time) >= time && !i->active) {
                i->active = true;
                i->Start();
            }
            if (i->duration != std::numeric_limits<double>::infinity() && ((i->time + i->duration) < time)) {
                if (p) {
                    p->next = i->next;
                } else {
                    head = i->next;
                }
                collected[collected_num++] = i;
            }
        }
        p = i;
    }
    for (size_t c = 0; c < collected_num; c++) {
        collected[c]->next = 0;
        collected[c]->Done();
    }
}

Timeline::Span &Timeline::Top(Span::Type type) const {
    static Timeline::Span empty;
    double time = SystemTime();
    for (Span *i = head; i ; i = i->next) {
        if ((i->type == type) &&
            (i->time <= time) &&
            ( (i->duration == std::numeric_limits<double>::infinity()) || ((i->time + i->duration) > time) ) ) {
            return *i;
        }
    }
    return empty;
}

Timeline::Span &Timeline::Below(Span *context, Span::Type type) const {
    static Timeline::Span empty;
    double time = SystemTime();
    for (Span *i = head; i ; i = i->next) {
        if (i == context) {
            continue;
        }
        if ((i->type == type) &&
            (i->time <= time) &&
            ( (i->duration == std::numeric_limits<double>::infinity()) || ((i->time + i->duration) > time) ) ) {
            return *i;
        }
    }
    return empty;
}

void Timeline::ProcessEffect()
{
    return Process(Span::Effect);
}

Timeline::Span &Timeline::TopEffect() const
{
    return Top(Span::Effect);
}

double Timeline::SystemTime() {
    return double(systemSeconds) + (double(GPT1->CNT) / double(GPT1->OCR[0]));
}

uint64_t Timeline::FastSystemTime() {
    return (uint64_t(systemSeconds) * uint64_t(GPT1->OCR[0])) + uint64_t(GPT1->CNT);
}

uint64_t Timeline::FastSystemTimeCmp() {
    return uint64_t(GPT1->CNT);
}

static bool idleReady = false;
static bool backgroundReady = false;

bool Timeline::CheckEffectReadyAndClear() {
    static size_t frameCount = 0;
    if (effectReady) {
		effectReady = false;
        idleReady = (frameCount % size_t(effectRate * idleRate)) == 0;
        backgroundReady = (frameCount % size_t(effectRate / backgroundRate)) == 0;
        frameCount ++;
        return true;
    }
    return false;
}

bool Timeline::CheckBackgroundReadyAndClear() {
    if (backgroundReady) {
        backgroundReady = false;
        return true;
    }
    return false;
}

bool Timeline::CheckIdleReadyAndClear() {
    if (idleReady) {
        idleReady = false;
        return true;
    }
    return false;
}

void Timeline::init() {
    __disable_irq();

    CLOCK_EnableClock(kCLOCK_Gpt1);

    CCM->CSCMR1 &= ~CCM_CSCMR1_PERCLK_CLK_SEL_MASK;
    CCM->CCGR1 |= CCM_CCGR1_CG10(3);

    GPT1->CR &= ~GPT_CR_EN_MASK;
    GPT1->CR |=  GPT_CR_SWR(1);
    while ( ( GPT1->CR & GPT_CR_SWR_MASK ) != 0) { }

    GPT1->PR = GPT_PR_PRESCALER(0) | GPT_PR_PRESCALER24M(0);
    GPT1->CR |= GPT_CR_CLKSRC(1) | GPT_CR_ENMOD(1) | GPT_CR_WAITEN(1) | GPT_CR_STOPEN(1);
    GPT1->OCR[0] = BOARD_BOOTCLOCKRUN_GPT1_IPG_CLK_HIGHFREQ;
    GPT1->OCR[1] = 0;
    GPT1->OCR[2] = 0;
    GPT1->IR = GPT_IR_OF1IE(1);
    GPT1->CR |= GPT_CR_EN(1);

    NVIC_SetPriority(GPT1_IRQn, 2);
    NVIC_EnableIRQ(GPT1_IRQn);

    CLOCK_EnableClock(kCLOCK_Gpt2);

    CCM->CSCMR1 &= ~CCM_CSCMR1_PERCLK_CLK_SEL_MASK;
    CCM->CCGR0 |= CCM_CCGR1_CG12(3);

    GPT2->CR &= ~GPT_CR_EN_MASK;
    GPT2->CR |=  GPT_CR_SWR(1);
    while ( ( GPT2->CR & GPT_CR_SWR_MASK ) != 0) { }

    GPT2->PR = GPT_PR_PRESCALER(0) | GPT_PR_PRESCALER24M(0);
    GPT2->CR |= GPT_CR_CLKSRC(1) | GPT_CR_ENMOD(1) | GPT_CR_WAITEN(1) | GPT_CR_STOPEN(1);
    GPT2->OCR[0] = BOARD_BOOTCLOCKRUN_GPT2_IPG_CLK_HIGHFREQ / effectRate;
    GPT2->OCR[1] = 0;
    GPT2->OCR[2] = 0;
    GPT2->IR = GPT_IR_OF1IE(1);
    GPT2->CR |= GPT_CR_EN(1);

    NVIC_SetPriority(GPT2_IRQn, 2);
    NVIC_EnableIRQ(GPT2_IRQn);

    __enable_irq();
}
