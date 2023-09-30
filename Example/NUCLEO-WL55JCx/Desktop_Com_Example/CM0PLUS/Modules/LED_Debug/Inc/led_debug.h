/*
 * For debug/testing purposes only! See note on blink_led().
 */

#ifndef LED_DEBUG_H_
#define LED_DEBUG_H_

#include "stm32wlxx_hal.h"
#include "stm32wlxx_hal_gpio.h"

typedef enum {
  RED_LED = GPIO_PIN_11,
  GREEN_LED = GPIO_PIN_9,
  BLUE_LED = GPIO_PIN_15
} LED_COLOR;


/* Blink LED in Sequence
 *
 * Adds a sequence characteristic to blink_led().
 *
 * Parameters:
 *  led - LED_COLOR enum of the LED color to blink.
 *  count - the number of times to blink the LED.
 *  delay - an integer corresponding to how long to delay between each blink (see note below).
 *  seq_count - the number of times to blink the LED sequence (count).
 */
void blink_led_sequence(LED_COLOR led, uint8_t count, uint64_t delay, uint8_t seq_count);


/* Blink LED
 *
 * Blinks the specified LED a set number of times with a delay.
 *
 * Parameters:
 *  led - LED_COLOR enum of the LED color to blink.
 *  count - the number of times to blink the LED.
 *  delay - an integer corresponding to how long to delay between each blink (see note below).
 *
 * Note: the delay is not in any unit of time but rather corresponds to the number of iterations
 *  an empty for loop runs. This means that the clock speed will affect the speed at which
 *  the LED blinks. This also means that compiler optimizations may cause unexpected behavior.
 *  This code has only been tested with no compiler optimizations (-O0 flag).
 */
void blink_led(LED_COLOR led, uint8_t count, uint64_t delay);


/* Activate LED
 *
 * Turns on an LED on the Nucleo board.
 *
 * Parameters:
 *  led - LED_COLOR enum of the LED color to activate.
 *
 * Note: will enable gpio port B and it's associated clock.
 */
void activate_led(LED_COLOR led);


/* Deactivate LED
 *
 * Parameters:
 *  led - LED_COLOR enum of the LED color to deactivate.
 *
 * Note: will disable gpio port B and it's associated clock if
 *  the port and clock was originally turned off before calling
 *  activate_led(). Behavior will only work correctly if activate_led()
 *  was called first.
 */
void deactivate_led(LED_COLOR led);


#endif /* LED_DEBUG_H_ */
