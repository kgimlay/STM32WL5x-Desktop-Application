#include "led_debug.h"

uint32_t gpioB_isEnabled = 0UL;


/* Blink LED in Sequence
 *
 * Adds a sequence characteristic to blink_led().
 */
void blink_led_sequence(LED_COLOR led, uint8_t count, uint64_t delay, uint8_t seq_count) {
  // Loop sequence count number of times, blinking the LED the number
  // of times specified with a pause between each loop.
  for (uint8_t sequence_counter = 0; sequence_counter < seq_count; sequence_counter += 1) {
    blink_led(led, count, delay);
    for (unsigned int i=0; i<65535*4; i++) {}
  }
}


/* Blink LED
 *
 * Blinks the specified LED a set number of times with a delay.
 */
void blink_led(LED_COLOR led, uint8_t count, uint64_t delay) {
  // Start the GPIO clock the LED is attached to.
  gpioB_isEnabled = __HAL_RCC_GPIOB_IS_CLK_ENABLED();
  if (!gpioB_isEnabled) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  }

  // Initialize the LED to blink.
  GPIO_InitTypeDef led_gpio_params = {
      .Pin = led,
      .Mode = GPIO_MODE_OUTPUT_PP,
      .Pull = GPIO_NOPULL,
      .Speed = GPIO_SPEED_FREQ_HIGH
  };
  HAL_GPIO_Init(GPIOB, &led_gpio_params);

  // Loop, toggling LED for count and delay specified.
  // Note: the delay will be altered by the clock speeds used in the MCU
  for (uint8_t toggle_counter = 0; toggle_counter < count; toggle_counter += 1) {
    HAL_GPIO_WritePin(GPIOB, led, GPIO_PIN_SET);
    for (unsigned int i=0; i<65535; i++) {}
    HAL_GPIO_WritePin(GPIOB, led, GPIO_PIN_RESET);
    for (unsigned int i=0; i<65535; i++) {}
  }

  // Deinitialize the LED used to  blink.
  HAL_GPIO_DeInit(GPIOB, led);

  // Disable the GPIO clock the LED is attached to.
  if (!gpioB_isEnabled) {
    __HAL_RCC_GPIOB_CLK_DISABLE();
  }
}


/* Activate LED
 *
 * Turns on an LED on the Nucleo board.
 */
void activate_led(LED_COLOR led) {
  // Start the GPIO clock the LED is attached to.
  gpioB_isEnabled = __HAL_RCC_GPIOB_IS_CLK_ENABLED();
  if (!gpioB_isEnabled) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  }

  // Initialize the LED to turn on.
  GPIO_InitTypeDef led_gpio_params = {
      .Pin = led,
      .Mode = GPIO_MODE_OUTPUT_PP,
      .Pull = GPIO_NOPULL,
      .Speed = GPIO_SPEED_FREQ_HIGH
  };
  HAL_GPIO_Init(GPIOB, &led_gpio_params);

  // Turn LED on.
  HAL_GPIO_WritePin(GPIOB, led, GPIO_PIN_SET);
}


/* Deactivare LED
 *
 * Turns off an LED on the Nucleo board.
 */
void deactivate_led(LED_COLOR led) {
  // Turn LED off.
  HAL_GPIO_WritePin(GPIOB, led, GPIO_PIN_RESET);

  // Deinitialize the LED.
  HAL_GPIO_DeInit(GPIOB, led);

  // Disable the GPIO clock the LED is attached to.
  if (!gpioB_isEnabled) {
    __HAL_RCC_GPIOB_CLK_DISABLE();
  }
}
