void ws2812b_color_joy(uint32_t counter) {
  // rgb timeout
  uint64_t latest_switch_ts = sw_timestamp[0];
  for (int i = 1; i < SW_GPIO_SIZE; i++)
    if (latest_switch_ts < sw_timestamp[i]) latest_switch_ts = sw_timestamp[i];
  if (time_us_64() > latest_switch_ts + timeout_us) {  // timed out
    for (int i = 0; i < WS2812B_LED_SIZE; i++) ws2812b_data[i] = COLOR_BLACK;
  } else {
    RGB_t base_color = COLOR_BASE_JOY;
    base_color.r *= WS2812B_BRIGHTNESS;
    base_color.g *= WS2812B_BRIGHTNESS;
    base_color.b *= WS2812B_BRIGHTNESS;
    /* Peripheral RGB */
    for (int i = (SW_GPIO_SIZE - 2) * 2; i < WS2812B_LED_SIZE; i++) {
      if (time_us_64() - reactive_timeout_timestamp >= REACTIVE_TIMEOUT_MAX) {
        ws2812b_data[i] = base_color;
      } else {
        ws2812b_data[i].r = lights_report.lights.rgb[0].r;
        ws2812b_data[i].g = lights_report.lights.rgb[0].g;
        ws2812b_data[i].b = lights_report.lights.rgb[0].b;
      }
    }
    /* Switches */
    for (int i = 0; i < SW_GPIO_SIZE - 3; i++) {
      if (time_us_64() - reactive_timeout_timestamp >= REACTIVE_TIMEOUT_MAX) {
        if ((report.buttons >> i) % 2 == 1) {
          ws2812b_data[2 * i + 2] = SW_COLORS_JOY[i + 1];
        } else {
          ws2812b_data[2 * i + 2] = COLOR_BLACK;
        }
      } else {
        if (lights_report.lights.buttons[i] == 0) {
          ws2812b_data[2 * i + 2] = COLOR_BLACK;
        } else {
          ws2812b_data[2 * i + 2] = SW_COLORS_JOY[i + 1];
        }
      }
    }
    /* start button sw_val index is offset by two with respect to LED_GPIO */
    if (time_us_64() - reactive_timeout_timestamp >= REACTIVE_TIMEOUT_MAX) {
      if ((report.buttons >> (SW_GPIO_SIZE - 1)) % 2 == 1) {
        ws2812b_data[0] = SW_COLORS_JOY[0];
      } else {
        ws2812b_data[0] = COLOR_BLACK;
      }
    } else {
      if (lights_report.lights.buttons[SW_GPIO_SIZE - 3] == 0) {
        ws2812b_data[0] = COLOR_BLACK;
      } else {
        ws2812b_data[0] = SW_COLORS_JOY[0];
      }
    }
    /* Switch Labels */
    for (int i = 0; i < SW_GPIO_SIZE - 2; i++) {
      ws2812b_data[2 * i + 1] = SW_LABEL_COLORS_JOY[i];
    }
    /* Knob Labels */
    ws2812b_data[18] = SW_KNOB_LABEL_COLORS_JOY[1]; // Right
    ws2812b_data[25] = SW_KNOB_LABEL_COLORS_JOY[0]; // Left

  }
  for (int i = 0; i < WS2812B_LED_SIZE; ++i) {
    put_pixel(
        urgb_u32(ws2812b_data[i].r, ws2812b_data[i].g, ws2812b_data[i].b));
  }
}