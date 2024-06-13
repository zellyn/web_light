#pragma once

#include <stdint.h>

void hsv2rgb(uint16_t h, uint16_t s, uint16_t v, uint16_t *r, uint16_t *g, uint16_t *b);

void rgb2hsv(uint16_t r, uint16_t g, uint16_t b, uint16_t *h, uint16_t *s, uint16_t *v);

uint16_t lerp(uint16_t v0, uint16_t v1, float t);

uint16_t hlerp(uint16_t h0, uint16_t h1, float t);
