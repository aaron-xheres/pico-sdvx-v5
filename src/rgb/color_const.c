const uint64_t timeout_us = 600000000;  // 600000000us = 10min
const double WS2812B_BRIGHTNESS = 0.2;

const RGB_t COLOR_BLACK = {0, 0, 0};
const RGB_t COLOR_BASE_JOY = {0, 0, 0};
const RGB_t COLOR_BASE_KB = {127, 0, 255};

/* Joystick Mode */
const RGB_t SW_LABEL_COLORS_JOY[] = {
    {64, 64, 64},  // START
    {0, 0, 255},  // BT-A
    {0, 0, 255},  // BT-B
    {0, 0, 255},  // BT-C
    {0, 0, 255},  // BT-D
    {128, 51, 0},  // FX-L
    {128, 51, 0},  // FX-R
};
const RGB_t SW_COLORS_JOY[] = {
    {0, 0, 255},      // START
    {255, 255, 255},  // BT-A
    {255, 255, 255},  // BT-B
    {255, 255, 255},  // BT-C
    {255, 255, 255},  // BT-D
    {255, 51, 0},     // FX-L
    {255, 51, 0},     // FX-R
};
const RGB_t SW_KNOB_LABEL_COLORS_JOY[] = {
    {0, 102, 255},  // Left
    {255, 0, 102},  // Right
};

/* Keyboard Mode */
const RGB_t SW_LABEL_COLORS_KB[] = {
    {255, 255, 255},  // START
    {255, 255, 255},  // BT-A
    {255, 255, 255},  // BT-B
    {255, 255, 255},  // BT-C
    {255, 255, 255},  // BT-D
    {255, 255, 255},  // FX-L
    {255, 255, 255},  // FX-R
};
const RGB_t SW_COLORS_KB[] = {
    {255, 51, 0},      // START
    {120, 234, 245},  // BT-A
    {245, 125, 189},  // BT-B
    {245, 125, 189},  // BT-C
    {120, 234, 245},  // BT-D
    {0, 102, 255},     // FX-L
    {0, 102, 255},     // FX-R
};

RGB_t ws2812b_data[WS2812B_LED_SIZE] = {0};