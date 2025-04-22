#ifndef CONTROLLER_CONFIG_H
#define CONTROLLER_CONFIG_H

#define SW_GPIO_SIZE 9                // Number of switches
#define LED_GPIO_SIZE 7               // Number of switch LEDs
#define ENC_GPIO_SIZE 2               // Number of encoders
#define ENC_PPR 24                    // Encoder PPR
#define MOUSE_SENS 5                  // Mouse sensitivity multiplier
#define ENC_DEBOUNCE true             // Encoder Debouncing
#define SW_DEBOUNCE_TIME_US 4000      // Switch debounce delay in us
#define ENC_PULSE (ENC_PPR * 4)       // 4 pulses per PPR
#define REACTIVE_TIMEOUT_MAX 1000000  // HID to reactive timeout in us
#define WS2812B_LED_SIZE 32           // Number of WS2812B LEDs
#define WS2812B_LED_ZONES 1           // Number of WS2812B LED Zones
#define WS2812B_LEDS_PER_ZONE \
  WS2812B_LED_SIZE / WS2812B_LED_ZONES  // Number of LEDs per zone

#ifdef PICO_GAME_CONTROLLER_C

// MODIFY KEYBINDS HERE, MAKE SURE LENGTHS MATCH SW_GPIO_SIZE
// { BT-A, BT-B, BT-C, BT-D, FX-L, FX-R, UNUSUED, EXTRA-BT, START}
const uint8_t SW_KEYCODE[] = {
    HID_KEY_S, HID_KEY_D, HID_KEY_K, HID_KEY_L, 
    HID_KEY_SHIFT_LEFT, HID_KEY_SHIFT_RIGHT, 
    HID_KEY_NONE, HID_KEY_NONE, HID_KEY_ENTER,
};

// Combination with Mod Key (back extra button)
const uint8_t SW_KEYCODE_MOD[] = {
  HID_KEY_ESCAPE, HID_KEY_NONE, HID_KEY_NONE, HID_KEY_DELETE, 
  HID_KEY_F1, HID_KEY_F2, 
  HID_KEY_NONE, HID_KEY_NONE, HID_KEY_F5,
};

// ENCODER -> KEYCODE 
const uint8_t ENC_KEYCODE_TURN_LEFT[] = { HID_KEY_ARROW_LEFT, HID_KEY_ARROW_UP };
const uint8_t ENC_KEYCODE_TURN_RIGHT[] = { HID_KEY_ARROW_RIGHT, HID_KEY_ARROW_DOWN };
const uint8_t ENC_KEYCODE_TURN_LEFT_MOD[] = { HID_KEY_NONE, HID_KEY_PAGE_UP };
const uint8_t ENC_KEYCODE_TURN_RIGHT_MOD[] = { HID_KEY_NONE, HID_KEY_PAGE_DOWN };

const uint8_t SW_GPIO[] = {
    29, 3, 4, 5, 17, 12, 0, 8, 1,
};
const uint8_t LED_GPIO[] = {21};
const uint8_t ENC_GPIO[] = {27, 6};     // L_ENC(0, 1); R_ENC(2, 3)
const bool ENC_REV[] = {false, false};  // Reverse Encoders
const uint8_t WS2812B_GPIO = 2;

#endif

extern bool joy_mode_check;

#endif