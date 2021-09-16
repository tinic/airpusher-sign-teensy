#include "timeline.h"
#include "ledpwm.h"
#include "sign.h"
#include "debugled.h"
#include "fsl_debug_console.h"

#include "MIMXRT1062.h"

static uint8_t stripBytes[1024];
static uint8_t brightness = 0x00;

void sign_entry() {
    Timeline::instance();
    LedsPWMDMA::instance();
    for(;;) {
        __WFI();
        //PRINTF("3 %f\r\n", Timeline::SystemTime());
        if (Timeline::instance().CheckIdleReadyAndClear()) {\
            // NOP
        }
        if (Timeline::instance().CheckBackgroundReadyAndClear()) {
        	// NOP
        }
        if (Timeline::instance().CheckEffectReadyAndClear()) {
        	//PRINTF("0!!!!\n\r");
            memset(stripBytes, brightness++, 1024);
            for (size_t c = 0; c < 6; c++) {
            	//PRINTF("1!!!!\n\r");
                LedsPWMDMA::instance().prepare(c, stripBytes);
            }
        	//PRINTF("2!!!!\n\r");
            LedsPWMDMA::instance().transfer();

        	Timeline::instance().ProcessEffect();
            if (Timeline::instance().TopEffect().Valid()) {
                Timeline::instance().TopEffect().Calc();
                Timeline::instance().TopEffect().Commit();
            }
        }
    }
}

