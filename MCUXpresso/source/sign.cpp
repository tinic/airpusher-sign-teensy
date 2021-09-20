#include "./timeline.h"
#include "./ledpwm.h"
#include "./leds.h"
#include "./effects.h"
#include "./sign.h"
#include "./debugled.h"

#include "fsl_debug_console.h"

#include "MIMXRT1062.h"

void sign_entry() {
    Timeline::instance();
    LedsPWMDMA::instance();
    Leds::instance();
    Effects::instance();
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
            LedsPWMDMA::instance().transfer();
        	Timeline::instance().ProcessInterval();
        	Timeline::instance().ProcessEffect();
            if (Timeline::instance().TopEffect().Valid()) {
                Timeline::instance().TopEffect().Calc();
                Timeline::instance().TopEffect().Commit();
            }
        }
    }
}

