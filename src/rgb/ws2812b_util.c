/*
 * ws2812b utility class
 * @author SpeedyPotato
 */
#include "ws2812.pio.h"

/**
 * WS2812B RGB Format Helper
 **/
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

/**
 * 768 Color Wheel Picker
 * @param wheel_pos Color value, r->g->b->r...
 **/
static inline uint32_t color_wheel(uint16_t wheel_pos) {
  wheel_pos %= 768;
  if (wheel_pos < 256) {
    return urgb_u32(wheel_pos, 255 - wheel_pos, 0);
  } else if (wheel_pos < 512) {
    wheel_pos -= 256;
    return urgb_u32(255 - wheel_pos, 0, wheel_pos);
  } else {
    wheel_pos -= 512;
    return urgb_u32(0, wheel_pos, 255 - wheel_pos);
  }
}

/**
 * 768 Color Wheel Picker
 * @param wheel_pos Color value, r->g->b->r...
 **/
static inline RGB_t color_wheel_rgbt(uint16_t wheel_pos) {
  wheel_pos %= 768;
  RGB_t init;
  if (wheel_pos < 256) {
    init.r = wheel_pos;
    init.g = 255 - wheel_pos;
    init.b = 0;
  } else if (wheel_pos < 512) {
    wheel_pos -= 256;
    init.r = 255 - wheel_pos;
    init.g = 0;
    init.b = wheel_pos;
  } else {
    wheel_pos -= 512;
    init.r = 0;
    init.g = wheel_pos;
    init.b = 255 - wheel_pos;
  };
  return init;
}

/**
 * WS2812B RGB Assignment
 * @param pixel_grb The pixel color to set
 **/
static inline void put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio1, ENC_GPIO_SIZE, pixel_grb << 8u);
}