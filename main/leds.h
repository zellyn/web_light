#pragma once

struct rgb {
  volatile uint8_t r;
  volatile uint8_t g;
  volatile uint8_t b;
};

extern struct rgb colors[];

volatile extern uint16_t fade_steps;
volatile extern uint16_t fade_step_ms;
volatile extern uint16_t stay_ms;

void run_weblight(void);

esp_err_t parse_hash_rrggbb_hex_color(char* color, struct rgb* rgb);
