#pragma once

struct rgb {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

extern struct rgb color1;
extern struct rgb color2;

extern bool fade;
extern uint16_t delay_ms;

void run_weblight(void);
