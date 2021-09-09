/*
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    airpusher-sign.cpp
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MIMXRT1062.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */

class LedsPWMDMA {
public:
	static constexpr size_t stripCount = 6;
	static constexpr size_t stripBytes = 1024;

    static LedsPWMDMA &instance();

    void prepare(size_t strip, const uint8_t *data);
    void transfer();
    void waitForTransfer();

	struct Cfg {
    	uint8_t  chn;
		volatile PWM_Type *pwm;
		uint8_t  sub;
		uint8_t  abx;
		uint32_t dmamux;
		uint32_t ctlmux;
		uint32_t altmode;
	};

	static const Cfg cfg[stripCount];

private:

    static void DMA0_IRQHandler();
    static void DMA1_IRQHandler();
    static void DMA2_IRQHandler();
    static void DMA3_IRQHandler();
    static void DMA4_IRQHandler();
    static void DMA5_IRQHandler();

    void setIRQVectors();

    static uint8_t pwmBuffer[stripBytes * stripCount * 8] __attribute__ ((aligned(32)));

    static_assert((1.25e-6 * double(BOARD_BOOTCLOCKRUN_IPG_CLK_ROOT)) <= 255.0);
    static constexpr uint8_t cmp_thl = uint8_t(1.25e-6 * double(BOARD_BOOTCLOCKRUN_IPG_CLK_ROOT));
    static constexpr uint8_t cmp_t0h = uint8_t(0.30e-6 * double(BOARD_BOOTCLOCKRUN_IPG_CLK_ROOT));
    static constexpr uint8_t cmp_t1h = uint8_t(0.75e-6 * double(BOARD_BOOTCLOCKRUN_IPG_CLK_ROOT));


    bool dmaInFlight = false;

    void init();
    bool initialized = false;
};

uint8_t LedsPWMDMA::pwmBuffer[stripBytes * stripCount * 8] __attribute__ ((aligned(32)));

const LedsPWMDMA::Cfg LedsPWMDMA::cfg[stripCount] = {
	//
	// Check IMXRT1062 reference manual for possible matches. Things are more constrained than they seem.
	// Main rules: - An individual PWM module can not use more than 3 distinct abx values.
	//             - PWM module + submodule combos have to be unique.
	// Guide: - Look for desired SW_MUX_CTL_PAD_GPIO_XXX pin in reference,
	//        - Find FLEXPWMxxxx ALT config and apply rules above to see if it will work.
	//
	// chn,  pwm, sub, abx, dmamux,                          ctlmux,                               altmode
	{    0, PWM2,   0,   0, kDmaRequestMuxFlexPWM2ValueSub0, kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06,   1     }, // Teensy 4.0 Pin 4
	{    1, PWM1,   3,   0, kDmaRequestMuxFlexPWM1ValueSub3, kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00,    6     }, // Teensy 4.0 Pin 8
	{    2, PWM2,   1,   1, kDmaRequestMuxFlexPWM2ValueSub1, kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08,   1     }, // Teensy 4.0 Pin 5
//	{    2, PWM1,   2,   2, kDmaRequestMuxFlexPWM1ValueSub2, kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_12, 4     }, // Teensy 4.0 Pin 24 / A10, underside
	{    3, PWM4,   0,   0, kDmaRequestMuxFlexPWM4ValueSub0, kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_08, 1     }, // Teensy 4.0 Pin 22 / A8
	{    4, PWM4,   1,   0, kDmaRequestMuxFlexPWM4ValueSub1, kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_09, 1     }, // Teensy 4.0 Pin 23 / A9
	{    5, PWM2,   2,   1, kDmaRequestMuxFlexPWM2ValueSub2, kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11,    2     }  // Teensy 4.0 Pin 9
	// Update transfer(), setIRQVectors() & DMAx_IRQHandler() if more entries are added
};

auto DMA_IRQHandler = [] (size_t c) {
	DMA0->CINT = LedsPWMDMA::cfg[c].chn; LedsPWMDMA::cfg[c].pwm->MCTRL &= PWM_MCTRL_RUN(~(1UL << LedsPWMDMA::cfg[c].sub));
};

void LedsPWMDMA::DMA0_IRQHandler() { DMA_IRQHandler(0); }
void LedsPWMDMA::DMA1_IRQHandler() { DMA_IRQHandler(1); }
void LedsPWMDMA::DMA2_IRQHandler() { DMA_IRQHandler(2); }
void LedsPWMDMA::DMA3_IRQHandler() { DMA_IRQHandler(3); }
void LedsPWMDMA::DMA4_IRQHandler() { DMA_IRQHandler(4); }
void LedsPWMDMA::DMA5_IRQHandler() { DMA_IRQHandler(5); }

void LedsPWMDMA::setIRQVectors() {
	NVIC_SetVector(IRQn_Type(DMA0_DMA16_IRQn + cfg[0].chn), uint32_t(DMA0_IRQHandler));
	NVIC_SetVector(IRQn_Type(DMA0_DMA16_IRQn + cfg[1].chn), uint32_t(DMA1_IRQHandler));
	NVIC_SetVector(IRQn_Type(DMA0_DMA16_IRQn + cfg[2].chn), uint32_t(DMA2_IRQHandler));
	NVIC_SetVector(IRQn_Type(DMA0_DMA16_IRQn + cfg[3].chn), uint32_t(DMA3_IRQHandler));
	NVIC_SetVector(IRQn_Type(DMA0_DMA16_IRQn + cfg[4].chn), uint32_t(DMA4_IRQHandler));
	NVIC_SetVector(IRQn_Type(DMA0_DMA16_IRQn + cfg[5].chn), uint32_t(DMA5_IRQHandler));
}

void LedsPWMDMA::transfer() {
	waitForTransfer();

	// Check LedsPWMDMA::cfg for (non-)matches
	PWM1->MCTRL |= PWM_MCTRL_RUN(15);
	PWM2->MCTRL |= PWM_MCTRL_RUN(15);
	PWM4->MCTRL |= PWM_MCTRL_RUN(15);

	dmaInFlight = true;
}

LedsPWMDMA &LedsPWMDMA::instance() {
    static LedsPWMDMA ledsPWMDMA;
    if (!ledsPWMDMA.initialized) {
    	ledsPWMDMA.initialized = true;
    	ledsPWMDMA.init();
    }
    return ledsPWMDMA;
}

void LedsPWMDMA::prepare(size_t strip, const uint8_t *data) {
	const uint8_t *src = data;
	uint8_t *dst = &pwmBuffer[stripBytes * strip * 8];

    auto convert_to_one_wire_pwm = [] (uint8_t *p, uint8_t v) {
        for (uint32_t b = 0; b < 8; b++) {
            if ( ((1<<(7-b)) & v) != 0 ) {
                *p++ = cmp_t1h;
            } else {
                *p++ = cmp_t0h;
            }
        }
        return p;
    };

    for (size_t c = 0; c < stripBytes; c++) {
    	dst = convert_to_one_wire_pwm(dst, *src++);
    }
}

void LedsPWMDMA::waitForTransfer() {
	if (!dmaInFlight) {
		for (size_t c = 0; c < stripCount; c++) {
			while (!(DMA0->TCD[cfg[c].chn].CSR & DMA_CSR_DONE(1))) {}
		}
		dmaInFlight = false;
	}
}

void LedsPWMDMA::init() {

	memset(pwmBuffer, 0, sizeof(pwmBuffer));

	// Mux PWM pins
	for (size_t c = 0; c < stripCount; c++) {
		IOMUXC->SW_MUX_CTL_PAD[cfg[c].ctlmux] = cfg[c].altmode;
	}

	auto configPWMModule = [=](volatile PWM_Type *pwm, uint8_t sub, uint8_t abx) {
		pwm->SM[sub].INIT = 0;
		pwm->SM[sub].VAL0 = 0;
		pwm->SM[sub].VAL1 = cmp_thl;
		pwm->SM[sub].VAL2 = 0;
		pwm->SM[sub].VAL3 = 0;
		pwm->SM[sub].VAL4 = 0;
		pwm->SM[sub].VAL5 = 0;
		switch (abx) {
			case 0: {
				pwm->OUTEN |= PWM_OUTEN_PWMA_EN(1);
			} break;
			case 1: {
				pwm->OUTEN |= PWM_OUTEN_PWMB_EN(1);
			} break;
			case 2: {
				pwm->SM[sub].OCTRL = PWM_OCTRL_POLX(1);
				pwm->OUTEN |= PWM_OUTEN_PWMX_EN(1);
			} break;
		}
		pwm->SM[sub].DMAEN = PWM_DMAEN_VALDE(1);
	};

	for (size_t c = 0; c < stripCount; c++) {
		configPWMModule(cfg[c].pwm,cfg[c].sub,cfg[c].abx);
	}

    auto resetDMAChannel = [] (uint32_t chn) {
        CCM->CCGR5 |= CCM_CCGR5_CG3(1); // DMA

        DMA0->CR = DMA_CR_GRP1PRI(1) | DMA_CR_EMLM(1) | DMA_CR_EDBG(1);

        DMA0->CERQ = chn;
        DMA0->CERR = chn;
        DMA0->CEEI = chn;
        DMA0->CINT = chn;

        memset(&DMA0->TCD[chn], 0, sizeof(DMA0->TCD[0]));
    };

    auto setDMATCD = [] (size_t c, size_t chn, volatile uint16_t *dst) {
		size_t len = stripBytes * 8;
		// FIXME, check fields!!!!
		DMA0->TCD[chn].SADDR 		= uint32_t(&pwmBuffer[len * c]);
		DMA0->TCD[chn].SOFF 	    = 1;
		DMA0->TCD[chn].ATTR      	= DMA_ATTR_SSIZE(1) | DMA_ATTR_DSIZE(2);
		DMA0->TCD[chn].NBYTES_MLNO	= 1;
		DMA0->TCD[chn].SLAST 		= -len;
		DMA0->TCD[chn].DADDR 		= uint32_t(dst);
		DMA0->TCD[chn].DOFF 		= 2;
		DMA0->TCD[chn].DLAST_SGA  	= -len * 2;
		DMA0->TCD[chn].BITER_ELINKNO = len;
		DMA0->TCD[chn].CITER_ELINKNO = len;
		DMA0->TCD[chn].CSR		   |= DMA_CSR_INTMAJOR(1);
    };

    setIRQVectors();

	for (size_t c = 0; c < stripCount; c++) {
		resetDMAChannel(cfg[c].chn);
		switch (cfg[c].abx) {
			case 0: {
				setDMATCD(c, cfg[c].chn, &cfg[c].pwm->SM[cfg[c].sub].VAL3);
			} break;
			case 1: {
				setDMATCD(c, cfg[c].chn, &cfg[c].pwm->SM[cfg[c].sub].VAL5);
			} break;
			case 2: {
				setDMATCD(c, cfg[c].chn, &cfg[c].pwm->SM[cfg[c].sub].VAL0);
			} break;
		}
        DMAMUX->CHCFG[cfg[c].chn] = 0;
        DMAMUX->CHCFG[cfg[c].chn] = (cfg[c].dmamux & 0x7F) | DMAMUX_CHCFG_ENBL(1);

        NVIC_EnableIRQ(IRQn_Type(DMA0_DMA16_IRQn + cfg[c].chn)); // Channel N

        DMA0->SERQ = cfg[c].chn;
	}
}

/*
 * @brief   Application entry point.
 */
int main(void) {

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();

#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    PRINTF("Hello World\n");

    uint8_t stripBytes[1024];
    uint8_t brightness = 0x7F;
    for (;;) {
		LedsPWMDMA::instance().waitForTransfer();
	    memset(stripBytes, brightness++, sizeof(stripBytes));
		for (int32_t c = 0; c < 6; c++) {
			LedsPWMDMA::instance().prepare(c, stripBytes);
		}
		LedsPWMDMA::instance().transfer();
    }

    /* Force the counter to be placed into memory. */
    volatile static int i = 0 ;
    /* Enter an infinite loop, just incrementing a counter. */
    while(1) {
        i++ ;
        /* 'Dummy' NOP to allow source level single stepping of
            tight while() loop */
        __asm volatile ("nop");
    }
    return 0 ;
}
