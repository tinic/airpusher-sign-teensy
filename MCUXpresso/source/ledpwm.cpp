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
#include "./ledpwm.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fsl_debug_console.h"
#include "fsl_clock.h"
#include "MIMXRT1062.h"

static inline void arm_dcache_flush_delete(void *addr, uint32_t size)
{
    __disable_irq();
    uint32_t location = (uint32_t)addr & 0xFFFFFFE0;
    uint32_t end_addr = (uint32_t)addr + size;
    __DSB();
    do {
        (*(volatile uint32_t *)0xE000EF70) = location;
        location += 32;
    } while (location < end_addr);
    __DSB();
    __ISB();
    __enable_irq();
}

__attribute__((section("DmaData"))) uint16_t LedsPWMDMA::pwmBuffer[pageCount][stripCount][stripBytes * 8 + frontTailPadding] __attribute__ ((aligned(32)));

const LedsPWMDMA::Cfg LedsPWMDMA::cfg[stripCount] = {
  //
  // Check IMXRT1062 reference manual for possible matches. Things are more constrained than they seem.
  // Main rules: - An individual PWM module can not use more than 3 distinct abx values.
  //             - Pins are limited to what abx value they can use
  //             - PWM module + submodule combos have to be unique.
  // Guide: - Look for desired SW_MUX_CTL_PAD_GPIO_XXX pin in reference,
  //        - Find FLEXPWMxxxx ALT config and apply rules above to see if it will work.
  //
  // chn,  pwm, sub, abx, dmamux,                          ctlmux,                               pwmmode
  {    0, PWM2,   0,   0, kDmaRequestMuxFlexPWM2ValueSub0, kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_06,   1       }, // Teensy 4.0 Pin 4
  {    1, PWM1,   3,   0, kDmaRequestMuxFlexPWM1ValueSub3, kIOMUXC_SW_MUX_CTL_PAD_GPIO_B1_00,    6       }, // Teensy 4.0 Pin 8
  {    2, PWM2,   1,   0, kDmaRequestMuxFlexPWM2ValueSub1, kIOMUXC_SW_MUX_CTL_PAD_GPIO_EMC_08,   1       }, // Teensy 4.0 Pin 5
  {    3, PWM4,   0,   0, kDmaRequestMuxFlexPWM4ValueSub0, kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_08, 1       }, // Teensy 4.0 Pin 22
  {    4, PWM4,   1,   0, kDmaRequestMuxFlexPWM4ValueSub1, kIOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_09, 1       }, // Teensy 4.0 Pin 23
  {    5, PWM2,   2,   1, kDmaRequestMuxFlexPWM2ValueSub2, kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_11,    2       }  // Teensy 4.0 Pin 9
  // Update transfer(), setIRQVectors() & DMAx_IRQHandler() if more entries are added
};

auto DMA_IRQHandler = [] (size_t c) {

    DMA0->CINT = LedsPWMDMA::cfg[c].chn;
    DMA0->CERQ = LedsPWMDMA::cfg[c].chn;
    DMA0->CERR = LedsPWMDMA::cfg[c].chn;
    DMA0->CEEI = LedsPWMDMA::cfg[c].chn;
    DMA0->CDNE = LedsPWMDMA::cfg[c].chn;

	IOMUXC->SW_MUX_CTL_PAD[LedsPWMDMA::cfg[c].ctlmux] = IOMUXC_SW_MUX_CTL_PAD_MUX_MODE(5);
    ((volatile PWM_Type*)LedsPWMDMA::cfg[c].pwm)->MCTRL &= ~PWM_MCTRL_RUN(1UL << LedsPWMDMA::cfg[c].sub);

    __DSB();
};

extern "C" __attribute__((used)) void DMA0_DMA16_IRQHandler() { DMA_IRQHandler(0); }
extern "C" __attribute__((used)) void DMA1_DMA17_IRQHandler() { DMA_IRQHandler(1); }
extern "C" __attribute__((used)) void DMA2_DMA18_IRQHandler() { DMA_IRQHandler(2); }
extern "C" __attribute__((used)) void DMA3_DMA19_IRQHandler() { DMA_IRQHandler(3); }
extern "C" __attribute__((used)) void DMA4_DMA20_IRQHandler() { DMA_IRQHandler(4); }
extern "C" __attribute__((used)) void DMA5_DMA21_IRQHandler() { DMA_IRQHandler(5); }

void LedsPWMDMA::transfer() {
    for (size_t c = 0; c < stripCount; c++) {
        IOMUXC->SW_MUX_CTL_PAD[cfg[c].ctlmux] = IOMUXC_SW_MUX_CTL_PAD_MUX_MODE(cfg[c].pwmmode);
    	DMA0->TCD[cfg[c].chn].SADDR = DMA_SADDR_SADDR(&pwmBuffer[currentPage][c][0]);
        DMA0->SERQ = cfg[c].chn;
        ((volatile PWM_Type*)LedsPWMDMA::cfg[c].pwm)->MCTRL |= PWM_MCTRL_RUN(1UL << LedsPWMDMA::cfg[c].sub);
    }
    currentPage %= 1;
}

LedsPWMDMA &LedsPWMDMA::instance() {
    static LedsPWMDMA ledsPWMDMA;
    if (!ledsPWMDMA.initialized) {
      ledsPWMDMA.initialized = true;
      ledsPWMDMA.init();
    }
    return ledsPWMDMA;
}

void __attribute__((section("RamFunction"))) LedsPWMDMA::prepare(size_t strip, const uint8_t *data, size_t len) {
    const uint8_t *src = data;
    uint16_t *dst = &pwmBuffer[currentPage][strip][0];
    uint16_t *stt = dst;

    for (size_t c = 0; c < frontTailPadding/2; c++) {
    	*dst++ = 0;
    }

    auto convert_to_one_wire_pwm = [] (uint16_t *p, uint8_t v) {
        for (uint32_t b = 0; b < 8; b++) {
            if ( ((1<<(7-b)) & v) != 0 ) {
                *p++ = cmp_t1h;
            } else {
                *p++ = cmp_t0h;
            }
        }
        return p;
    };

    for (size_t c = 0; c < len; c++) {
       dst = convert_to_one_wire_pwm(dst, *src++);
    }

    for (size_t c = 0; c < frontTailPadding/2; c++) {
    	*dst++ = 0;
    }

    arm_dcache_flush_delete(&pwmBuffer[currentPage][strip][0], dst-stt);
}

void LedsPWMDMA::resetHardware() {

    CLOCK_EnableClock(kCLOCK_Dma);

    CLOCK_EnableClock(kCLOCK_Pwm1);
    CLOCK_EnableClock(kCLOCK_Pwm2);
    CLOCK_EnableClock(kCLOCK_Pwm3);
    CLOCK_EnableClock(kCLOCK_Pwm4);

	memset(pwmBuffer, 0, sizeof(pwmBuffer));

	auto resetPWMModule = [=](volatile PWM_Type *pwm) {
		pwm->FCTRL = PWM_FCTRL_FLVL(0xF); // logic high = fault
		pwm->FSTS = 0x000F; // clear fault status
		pwm->FFILT = 0;
		pwm->MCTRL |= PWM_MCTRL_CLDOK(15);
		auto resetSubModule = [=](volatile PWM_Type *pwm, uint8_t sub) {
			pwm->SM[sub].CTRL2 = PWM_CTRL2_INDEP(1) | PWM_CTRL2_WAITEN(1) | PWM_CTRL2_DBGEN(1);
			pwm->SM[sub].CTRL = PWM_CTRL_FULL(1);
			pwm->SM[sub].OCTRL = 0;
			pwm->SM[sub].DTCNT0 = 0;
			pwm->SM[sub].INIT = 0;
			pwm->SM[sub].VAL0 = 0;
			pwm->SM[sub].VAL1 = 32768;
			pwm->SM[sub].VAL2 = 0;
			pwm->SM[sub].VAL3 = 0;
			pwm->SM[sub].VAL4 = 0;
			pwm->SM[sub].VAL5 = 0;
		};
	    for (size_t c = 0; c < 4; c++) {
	    	resetSubModule(pwm, c);
	    }
	    pwm->MCTRL |= PWM_MCTRL_LDOK(15);
	    pwm->MCTRL |= PWM_MCTRL_RUN(15);
	};

	resetPWMModule(PWM1);
	resetPWMModule(PWM2);
	resetPWMModule(PWM3);
	resetPWMModule(PWM4);
}

void LedsPWMDMA::init() {
    __disable_irq();

    resetHardware();

    // Mux PWM pins
    for (size_t c = 0; c < stripCount; c++) {
        IOMUXC->SW_MUX_CTL_PAD[cfg[c].ctlmux] = IOMUXC_SW_MUX_CTL_PAD_MUX_MODE(cfg[c].pwmmode);
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
            pwm->OUTEN |= PWM_OUTEN_PWMA_EN(1UL << sub);
          } break;
          case 1: {
            pwm->OUTEN |= PWM_OUTEN_PWMB_EN(1UL << sub);
          } break;
          case 2: {
            pwm->SM[sub].OCTRL = PWM_OCTRL_POLX(1);
            pwm->OUTEN |= PWM_OUTEN_PWMX_EN(1UL << sub);
          } break;
        }
        pwm->SM[sub].DMAEN  = PWM_DMAEN_VALDE(1);

//        pwm->SWCOUT = 0;
//        pwm->DTSRCSEL = 0xAAAA;
//        pwm->SM[sub].CTRL2 &= (~PWM_CTRL2_FORCE_SEL_MASK) & (~PWM_CTRL2_FORCE_MASK);
    };

    for (size_t c = 0; c < stripCount; c++) {
        configPWMModule(((volatile PWM_Type*)cfg[c].pwm),cfg[c].sub,cfg[c].abx);
    }


    CCM->CCGR5 |= CCM_CCGR5_CG3(1); // DMA

    DMA0->CR = DMA_CR_GRP1PRI(1) | DMA_CR_EDBG(1) | DMA_CR_EMLM(1);

    auto resetDMAChannel = [] (uint32_t chn) {

        DMA0->CERQ = chn;
        DMA0->CERR = chn;
        DMA0->CEEI = chn;
        DMA0->CINT = chn;
        DMA0->CDNE = chn;

        memset(&DMA0->TCD[chn], 0, sizeof(DMA0->TCD[0]));
    };

    auto setDMATCD = [] (size_t c, size_t chn, size_t page, volatile uint16_t *dst) {
        size_t len = sizeof(pwmBuffer[0][0]);

        DMA0->TCD[chn].SADDR            = DMA_SADDR_SADDR(&pwmBuffer[page][c][0]);
        DMA0->TCD[chn].DADDR            = DMA_DADDR_DADDR(dst);

        DMA0->TCD[chn].ATTR             = DMA_ATTR_SSIZE(1) | DMA_ATTR_DSIZE(1);

        DMA0->TCD[chn].SOFF             = DMA_SOFF_SOFF(2);
        DMA0->TCD[chn].DOFF             = DMA_DOFF_DOFF(0);

        DMA0->TCD[chn].SLAST            = DMA_SLAST_SLAST(-len);
        DMA0->TCD[chn].DLAST_SGA        = DMA_DLAST_SGA_DLASTSGA(0);

        DMA0->TCD[chn].BITER_ELINKNO    = DMA_BITER_ELINKNO_BITER(len / 2);
        DMA0->TCD[chn].CITER_ELINKNO    = DMA_BITER_ELINKNO_BITER(len / 2);

        DMA0->TCD[chn].NBYTES_MLNO      = DMA_NBYTES_MLNO_NBYTES(2);

        DMA0->TCD[chn].CSR              = DMA_CSR_INTMAJOR(1);
    };

    for (size_t c = 0; c < stripCount; c++) {
        resetDMAChannel(cfg[c].chn);
    }

    for (size_t c = 0; c < stripCount; c++) {
        switch (cfg[c].abx) {
            case 0: {
                setDMATCD(c, cfg[c].chn, currentPage, &((volatile PWM_Type*)cfg[c].pwm)->SM[cfg[c].sub].VAL3);
            } break;
            case 1: {
                setDMATCD(c, cfg[c].chn, currentPage, &((volatile PWM_Type*)cfg[c].pwm)->SM[cfg[c].sub].VAL5);
            } break;
            case 2: {
                setDMATCD(c, cfg[c].chn, currentPage, &((volatile PWM_Type*)cfg[c].pwm)->SM[cfg[c].sub].VAL0);
            } break;
        }

        DMAMUX->CHCFG[cfg[c].chn] = 0;
        DMAMUX->CHCFG[cfg[c].chn] = (cfg[c].dmamux & 0x7F) | DMAMUX_CHCFG_ENBL(1);

        NVIC_SetPriority(IRQn_Type(DMA0_DMA16_IRQn + cfg[c].chn), 1);
        NVIC_EnableIRQ(IRQn_Type(DMA0_DMA16_IRQn + cfg[c].chn)); // Channel N

        DMA0->SERQ = cfg[c].chn;
    }
    __enable_irq();
}

