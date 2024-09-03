#ifndef IRIS_BOARD1_H
#define IRIS_BOARD1_H

#ifdef __cplusplus
extern "C" {
#endif

#include <hal/nrf_gpio.h>

/**GENERIC**/
#define PIN_SDA    19
#define PIN_SCL    13
#define PIN_NFC1   9
#define PIN_NFC2   10
#define PIN_NRESET 18
#define PIN_2      2
#define PIN_4      4
#define PIN_6      6
#define PIN_8      8

/*BUTTONS*/
#define BUTTONS_NUMBER       0
#define BUTTONS_LIST         {}
#define BUTTONS_ACTIVE_STATE 0
#define BUTTON_PULL          NRF_GPIO_PIN_PULLUP

/*LEDS*/
#define LEDS_NUMBER       3
#define LED_1             4
#define LED_2             27
#define LED_3             26
#define LED_RED           LED_1
#define LED_GREEN         LED_2
#define LED_BLUE          LED_3
#define LEDS_LIST         {LED_RED, LED_GREEN, LED_BLUE}
#define LEDS_ACTIVE_STATE 0

/**FXOS8700CQ**/
/**wrong?**/
#define FXOS8700CQ_PRESENT  1
#define PIN_INT1_FXOS8700CQ 25
#define PIN_INT2_FXOS8700CQ 24
#define PIN_RST_FXOS8700CQ  23

/**LPS25HBTR**/
#define LPS25HBTR_PRESENT     1
#define PIN_INT_DRD_LPS25HBTR 13

/**HTS221**/
#define HTS221_PRESENT  1
#define PIN_DRDY_HTS221 8

#ifdef __cplusplus
}
#endif

#endif // IRIS_BOARD1_H
