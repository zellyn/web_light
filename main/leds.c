#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "esp_err.h"
#include "esp_log.h"
#include "led_strip.h"

#include "weblight.h"
#include "leds.h"

// GPIO assignment
#define LED_STRIP_BLINK_GPIO  2
// Numbers of the LED in the strip
#define LED_STRIP_LED_NUMBERS 5
// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)
// Delay between loops
#define CHASE_SPEED_MS        100

struct rgb colors[] = {
  {0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00},
};

volatile uint16_t fade_steps = 100;
volatile uint16_t fade_step_ms = 1;
volatile uint16_t stay_ms = 5000;

uint16_t clamp(uint16_t min, uint16_t max, uint16_t val) {
  if (val < min) return min;
  if (val > max) return max;
  return val;
}

// Get uint8_t value of hex digit, or 0, if it's not a valid hex digit.
uint8_t char2hex(char c) {
  if (c < '0') return 0;
  if (c <= '9') return (uint8_t)(c-'0');
  c = toupper(c);
  if (c < 'A') return 0;
  if (c <= 'F') return (uint8_t)(c-'A'+10);
  return 0;
}

esp_err_t parse_hash_rrggbb_hex_color(char* color, struct rgb* rgb) {
  ESP_LOGI(TAG, "Parsing hex color '%s'", color);
 if (strnlen(color, 8) != 7) {
   ESP_LOGI(TAG, "Want size 7; got %d", strnlen(color, 8));
    return ESP_ERR_INVALID_SIZE;
  }

  if (*color != '#') {
    ESP_LOGI(TAG, "Want first char of '#'; got %c", *color);
    return ESP_ERR_INVALID_ARG;
  }

  for (size_t i=1; i <7; i++) {
    if (!isxdigit((int)(color[i]))) {
      ESP_LOGI(TAG, "Bad hex digit: %c", color[i]);
      return ESP_ERR_INVALID_ARG;
    }
  }

  rgb->r = char2hex(color[2]);
  rgb->r += char2hex(color[1]) << 4;
  rgb->g = char2hex(color[4]);
  rgb->g += char2hex(color[3]) << 4;
  rgb->b = char2hex(color[6]);
  rgb->b += char2hex(color[5]) << 4;

  return ESP_OK;
}




led_strip_handle_t configure_led(void)
{
    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_BLINK_GPIO,   // The GPIO that connected to the LED strip's data line
        .max_leds = LED_STRIP_LED_NUMBERS,        // The number of LEDs in the strip,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
        .led_model = LED_MODEL_WS2812,            // LED strip model
        .flags.invert_out = false,                // whether to invert the output signal
    };

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
        .rmt_channel = 0,
#else
        .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
        .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
        .flags.with_dma = false,               // DMA feature is available on ESP target like ESP32-S3
#endif
    };

    // LED Strip object handle
    led_strip_handle_t led_strip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");
    return led_strip;
}

void run_weblight(void)
{
    led_strip_handle_t led_strip = configure_led();
    // uint16_t hue = 0;

    ESP_LOGI(TAG, "Start blinking LED strip");

    int cur = 0;
    while(1) {
      cur = cur^1;

      for (int i = 0; i < LED_STRIP_LED_NUMBERS; i++) {
	ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, colors[cur].g, colors[cur].r, colors[cur].b));
	ESP_ERROR_CHECK(led_strip_refresh(led_strip));
	// hue = (hue+4)%360;
	// ESP_ERROR_CHECK(led_strip_set_pixel_hsv(led_strip, i, hue, 255, 255));
	// ESP_ERROR_CHECK(led_strip_refresh(led_strip));
	// ESP_ERROR_CHECK(led_strip_set_pixel_hsv(led_strip, i, 0, 0, 0));
	// vTaskDelay(pdMS_TO_TICKS(CHASE_SPEED_MS));
      }
      vTaskDelay(pdMS_TO_TICKS(stay_ms));
    }
}
