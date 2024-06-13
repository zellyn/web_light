#include "hsv.h"

/**
 * @brief Simple helper function, converting HSV color space to RGB color space
 *
 * Wiki: https://en.wikipedia.org/wiki/HSL_and_HSV
 *
 * h: [0,360)
 * s, v: [0,255] (corresponding to (0.0,1.0)
 * r, g, b: [0,255]
 *
 */
void hsv2rgb(uint16_t h, uint16_t s, uint16_t v, uint16_t *r, uint16_t *g, uint16_t *b)
{
  h %= 360; // h -> [0,360)
  uint16_t rgb_max = v;
  uint16_t rgb_min = rgb_max * (255 - s) / 255.0f;

  uint16_t i = h / 60;
  uint16_t diff = h % 60;

  // RGB adjustment amount by hue
  uint16_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

  switch (i) {
  case 0:
    *r = rgb_max;
    *g = rgb_min + rgb_adj;
    *b = rgb_min;
    break;
  case 1:
    *r = rgb_max - rgb_adj;
    *g = rgb_max;
    *b = rgb_min;
    break;
  case 2:
    *r = rgb_min;
    *g = rgb_max;
    *b = rgb_min + rgb_adj;
    break;
  case 3:
    *r = rgb_min;
    *g = rgb_max - rgb_adj;
    *b = rgb_max;
    break;
  case 4:
    *r = rgb_min + rgb_adj;
    *g = rgb_min;
    *b = rgb_max;
    break;
  default:
    *r = rgb_max;
    *g = rgb_min;
    *b = rgb_max - rgb_adj;
    break;
  }
}

/**
 * @brief Simple helper function, converting RGB color space to HSV color space
 *
 * Wiki: https://en.wikipedia.org/wiki/HSL_and_HSV
 *
 * r, g, b: [0,255]
 * h: [0,360)
 * s, v: [0,255] (corresponding to (0.0,1.0)
 *
 */
void rgb2hsv(uint16_t r, uint16_t g, uint16_t b, uint16_t *h, uint16_t *s, uint16_t *v)
{
  uint16_t xmax = r;
  uint16_t xmin = r;
  if (g > xmax) xmax = g;
  if (b > xmax) xmax = b;
  if (g < xmin) xmin = g;
  if (b < xmin) xmin = b;

  uint16_t c = xmax-xmin;

  *v = xmax;

  if (xmax == 0 || c == 0) {
    *h = 0;
    *s = 0;
    return;
  }

  *s = c * 255 / xmax;

  if (xmax == r) {
    *h = (g>b) ? (60*(g-b)/c) : (360-60*(b-g)/c);
  } else if (xmax == g) {
    *h = (b>r) ? (120+60*(b-r)/c) : (120-60*(r-b)/c);
  } else {
    *h = (r>g) ? (240+60*(r-g)/c) : (240-60*(g-r)/c);
  }
}

// Lerp between two uint16_t values.
uint16_t lerp(uint16_t v0, uint16_t v1, float t) {
  if (t < 0.0) return v0;
  if (t > 1.0) return v1;
  if (v1 == v0) return v0;
  if (v1 > v0) {
    return v0 + (uint16_t) ((float)(v1-v0) * t);
  }
  return v0 - (uint16_t) ((float)(v0-v1) * t);
}

// Lerp between two H (as in HSV) angles, in the nearest direction.
uint16_t hlerp(uint16_t h0, uint16_t h1, float t) {
  if (t < 0.0) return h0;
  if (t > 1.0) return h1;
  if (h0 == h1) return h0;

  if (h1 > h0) {
    if (h1-h0 <= 180) {
      return h0 + (uint16_t) ((float)(h1-h0) * t);
    }

    return (h0 + 360 - (uint16_t) ((float)(h0+360-h1) * t)) % 360;
  }

  if (h0-h1 <= 180) {
    return h0 - (uint16_t) ((float)(h0-h1) * t);
  }

  return (h0 + (uint16_t) ((float)(h1+360-h0) * t)) % 360;
}
