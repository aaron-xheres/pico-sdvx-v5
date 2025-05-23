/*
 * Pico Game Controller
 * @author SpeedyPotato
 */
#define PICO_GAME_CONTROLLER_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
#include "bsp/board.h"
#include "controller_config.h"
#include "types.h"
#include "encoders.pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "debounce/debounce_include.h"
#include "rgb/rgb_include.h"
// clang-format on

PIO pio, pio_1;
uint32_t enc_val[ENC_GPIO_SIZE];
uint32_t prev_enc_val[ENC_GPIO_SIZE];
int cur_enc_val[ENC_GPIO_SIZE];

bool prev_sw_val[SW_GPIO_SIZE];
uint64_t sw_timestamp[SW_GPIO_SIZE];

bool kbm_report;
bool joy_kbm_report = false;

uint64_t reactive_timeout_timestamp;

void (*ws2812b_mode)();
void (*loop_mode)();
uint16_t (*debounce_mode)();
bool joy_mode_check = true;

lights_report_t lights_report;
report_t report;

/**
 * Gamepad Mode
 **/
void joy_mode() {
  if (tud_hid_ready()) {
    // find the delta between previous and current enc_val
    for (int i = 0; i < ENC_GPIO_SIZE; i++) {
      cur_enc_val[i] +=
          ((ENC_REV[i] ? 1 : -1) * (enc_val[i] - prev_enc_val[i]));
      while (cur_enc_val[i] < 0) cur_enc_val[i] = ENC_PULSE + cur_enc_val[i];
      cur_enc_val[i] %= ENC_PULSE;

      prev_enc_val[i] = enc_val[i];
    }

    report.joy0 = ((double)cur_enc_val[0] / ENC_PULSE) * (UINT8_MAX + 1);
    report.joy1 = ((double)cur_enc_val[1] / ENC_PULSE) * (UINT8_MAX + 1);

    // Bind the spare button to Escape
    if((report.buttons >> 6) % 2 == 1) {
      uint8_t nkro_report[32] = {0};
      uint8_t bit = HID_KEY_ESCAPE % 8;
      uint8_t byte = (HID_KEY_ESCAPE / 8) + 1;
      nkro_report[byte] |= (1 << bit);
      joy_kbm_report = true;
      tud_hid_n_report(0x00, REPORT_ID_KEYBOARD, &nkro_report, sizeof(nkro_report));
      return;
    }

    if (joy_kbm_report && (report.buttons >> 6) % 2 == 0) {
      uint8_t nkro_report[32] = {0};
      tud_hid_n_report(0x00, REPORT_ID_KEYBOARD, &nkro_report, sizeof(nkro_report));
      joy_kbm_report = false;
    } else {
      tud_hid_n_report(0x00, REPORT_ID_JOYSTICK, &report, sizeof(report));
    }
  }
}

/**
 * Keyboard Mode
 **/
bool is_holding_mod() {
  return (report.buttons >> 6) % 2 == 1;
}

void key_mode() {
  if (tud_hid_ready()) {  // Wait for ready, updating mouse too fast hampers movement
    /*------------- Keyboard -------------*/
    uint8_t nkro_report[32] = {0};

    for (int i = 0; i < SW_GPIO_SIZE; i++) {
      if ((report.buttons >> i) % 2 == 1) {
        uint8_t keycode;
        if (is_holding_mod()) {
          keycode = SW_KEYCODE_MOD[i];
        } else {
          keycode = SW_KEYCODE[i];
        }

        uint8_t bit = keycode % 8;
        uint8_t byte = (keycode / 8) + 1;
        if (keycode >= 240 && keycode <= 247) {
          nkro_report[0] |= (1 << bit);
        } else if (byte > 0 && byte <= 31) {
          nkro_report[byte] |= (1 << bit);
        }
      }
    }

    // Handle Encoders -> Key Presses
    for (int i = 0; i < ENC_GPIO_SIZE; i++) {
      if (enc_val[i] != prev_enc_val[i]) {
        uint8_t keycode;
        uint8_t bit, byte;
        int delta = (enc_val[i] - prev_enc_val[i]) * (ENC_REV[i] ? 1 : -1);

        if (abs(delta) < ENC_KEYCODE_READ_THRESHOLD) {
          continue;
        }

        if (delta < 0) {
          if (is_holding_mod()) {
            keycode = ENC_KEYCODE_TURN_LEFT_MOD[i];
          } else {
            keycode = ENC_KEYCODE_TURN_LEFT[i];
          }
        } else if (delta > 0) {
          if (is_holding_mod()) {
            keycode = ENC_KEYCODE_TURN_RIGHT_MOD[i];
          } else {
            keycode = ENC_KEYCODE_TURN_RIGHT[i];
          }
        }

        bit = keycode % 8;
        byte = (keycode / 8) + 1;
        if (keycode >= 240 && keycode <= 247) {
          nkro_report[0] |= (1 << bit);
        } else if (byte > 0 && byte <= 31) {
          nkro_report[byte] |= (1 << bit);
        }
      }

      prev_enc_val[i] = enc_val[i];
    }

    tud_hid_n_report(0x00, REPORT_ID_KEYBOARD, &nkro_report,
                      sizeof(nkro_report));
    
    /*------------- Mouse -------------*/
    // find the delta between previous and current enc_val
    // int delta[ENC_GPIO_SIZE] = {0};
    // for (int i = 0; i < ENC_GPIO_SIZE; i++) {
    //   delta[i] = (enc_val[i] - prev_enc_val[i]) * (ENC_REV[i] ? 1 : -1);
    //   prev_enc_val[i] = enc_val[i];
    // }

    // tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta[0] * MOUSE_SENS,
    //                      delta[1] * MOUSE_SENS, 0, 0);
  }
}

/**
 * Update Input States
 * Note: Switches are pull up, negate value
 **/
void update_inputs() {
  for (int i = 0; i < SW_GPIO_SIZE; i++) {
    // If switch gets pressed, record timestamp
    if (prev_sw_val[i] == false && !gpio_get(SW_GPIO[i]) == true) {
      sw_timestamp[i] = time_us_64();
    }
    prev_sw_val[i] = !gpio_get(SW_GPIO[i]);
  }
}

/**
 * DMA Encoder Logic For 2 Encoders
 **/
void dma_handler() {
  uint i = 1;
  int interrupt_channel = 0;
  while ((i & dma_hw->ints0) == 0) {
    i = i << 1;
    ++interrupt_channel;
  }
  dma_hw->ints0 = 1u << interrupt_channel;
  if (interrupt_channel < 4) {
    dma_channel_set_read_addr(interrupt_channel, &pio->rxf[interrupt_channel],
                              true);
  }
}

/**
 * Second Core Runnable
 **/
void core1_entry() {
  uint32_t counter = 0;
  uint32_t rgb_idx = 0;
  while (1) {
    counter++;
    if (counter % 32 == 0) {
      rgb_idx = ++rgb_idx % 768;
      ws2812b_mode(rgb_idx);
    }
    sleep_ms(1);
  }
}

/**
 * Initialize Board Pins
 **/
void init() {
  // LED Pin on when connected
  gpio_init(25);
  gpio_set_dir(25, GPIO_OUT);
  gpio_put(25, 1);

  // Set up the state machine for encoders
  pio = pio0;
  uint offset = pio_add_program(pio, &encoders_program);

  // Setup Encoders
  for (int i = 0; i < ENC_GPIO_SIZE; i++) {
    enc_val[i], prev_enc_val[i], cur_enc_val[i] = 0;
    encoders_program_init(pio, i, offset, ENC_GPIO[i], ENC_DEBOUNCE);

    dma_channel_config c = dma_channel_get_default_config(i);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(pio, i, false));

    dma_channel_configure(i, &c,
                          &enc_val[i],   // Destination pointer
                          &pio->rxf[i],  // Source pointer
                          0x10,          // Number of transfers
                          true           // Start immediately
    );
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    dma_channel_set_irq0_enabled(i, true);
  }

  reactive_timeout_timestamp = time_us_64();

  // Set up WS2812B
  pio_1 = pio1;
  uint offset2 = pio_add_program(pio_1, &ws2812_program);
  ws2812_program_init(pio_1, ENC_GPIO_SIZE, offset2, WS2812B_GPIO, 800000,
                      false);

  // Setup Button GPIO
  for (int i = 0; i < SW_GPIO_SIZE; i++) {
    prev_sw_val[i] = false;
    sw_timestamp[i] = 0;
    gpio_init(SW_GPIO[i]);
    gpio_set_function(SW_GPIO[i], GPIO_FUNC_SIO);
    gpio_set_dir(SW_GPIO[i], GPIO_IN);
    gpio_pull_up(SW_GPIO[i]);
  }

  // Set listener bools
  kbm_report = false;

  // Joy/KB Mode Switching
  if (!gpio_get(SW_GPIO[0])) {
    loop_mode = &key_mode;
    joy_mode_check = false;
  } else {
    loop_mode = &joy_mode;
    joy_mode_check = true;
  }

  if (joy_mode_check) {
    ws2812b_mode = &ws2812b_color_joy;
  } else {
    ws2812b_mode = &ws2812b_color_kb;
  }


  // Debouncing Mode
  debounce_mode = &debounce_eager;

  // Disable RGB
  if (gpio_get(SW_GPIO[8])) {
    multicore_launch_core1(core1_entry);
  }
}

/**
 * Main Loop Function
 **/
int main(void) {
  board_init();
  init();
  tusb_init();

  while (1) {
    tud_task();  // tinyusb device task
    update_inputs();
    report.buttons = debounce_mode();
    loop_mode();
  }

  return 0;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t* buffer,
                               uint16_t reqlen) {
  // TODO not Implemented
  (void)itf;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const* buffer,
                           uint16_t bufsize) {
  (void)itf;
  if (report_id == 2 && report_type == HID_REPORT_TYPE_OUTPUT &&
      bufsize >= sizeof(lights_report))  // light data
  {
    size_t i = 0;
    for (i; i < sizeof(lights_report); i++) {
      lights_report.raw[i] = buffer[i];
    }
    reactive_timeout_timestamp = time_us_64();
  }
}
