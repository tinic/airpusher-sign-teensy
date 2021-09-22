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
#ifndef _LED_PWM_H_
#define _LED_PWM_H_

#include <stdint.h>
#include <stdlib.h>

#include "clock_config.h"
#include "leds.h"

class LedsPWMDMA {
public:
	static constexpr size_t maxLeds = Leds::maxLedsPerPort;
	static constexpr size_t portCount = Leds::maxPorts;
	static constexpr size_t maxLedBytes = Leds::maxBytesPerLed;
	static constexpr size_t stripBytes = (maxLeds * maxLedBytes + 31) & (~31);

    static LedsPWMDMA &instance();

    void prepare(size_t port, const uint8_t *data, size_t len);
    void transfer();

	struct Cfg {
		uint8_t  chn;
		volatile void *pwm;
		uint8_t  sub;
		uint8_t  abx;
		uint32_t dmamux;
		uint32_t ctlmux;
		uint32_t pwmmode;
	};

	static const Cfg cfg[portCount];

  	static constexpr size_t pageCount = 2;
	static constexpr size_t frontTailPadding = 256;

    static __attribute__((section("DmaData"))) uint16_t pwmBuffer[pageCount][portCount][stripBytes * 8 + frontTailPadding] __attribute__ ((aligned(32)));

    static constexpr uint8_t cmp_thl = uint8_t(1.25e-6 * double(BOARD_BOOTCLOCKRUN_IPG_CLK_ROOT));
    static constexpr uint8_t cmp_t0h = uint8_t(0.35e-6 * double(BOARD_BOOTCLOCKRUN_IPG_CLK_ROOT));
    static constexpr uint8_t cmp_t1h = uint8_t(0.70e-6 * double(BOARD_BOOTCLOCKRUN_IPG_CLK_ROOT));

    void resetHardware();
    void init();
    bool initialized = false;
    size_t currentPage = 0;
};

#endif  // #ifndef _LED_PWM_H_
