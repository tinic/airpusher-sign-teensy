#include "MIMXRT1062.h"

void debug_led(int state) {
    __disable_irq();
	IOMUXC->SW_MUX_CTL_PAD[kIOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03] = IOMUXC_SW_MUX_CTL_PAD_MUX_MODE(5);
	IOMUXC->SW_PAD_CTL_PAD[kIOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03] = IOMUXC_SW_PAD_CTL_PAD_DSE(7);
	IOMUXC_GPR->GPR27 = IOMUXC_GPR_GPR27_GPIO_MUX2_GPIO_SEL(0xFFFFFFFF);
	GPIO7->GDIR |= (1u << 3);
	if (state) {
		GPIO7->DR_SET |= (1u << 3);
	} else {
		GPIO7->DR_CLEAR |= (1u << 3);
	}
    __enable_irq();
}
