/**
 *  \file
 *
 *  \brief User interface
 *
 *  \author Szymon Kulis, CERN, Geneva
 */

#include "ui.h"
#include "astring.h"
#include "cli.h"
#include "cmdarg.h"
#include "config_file.h"
#include "debug.h"
#include "led.h"
#include "mt.h"
#include "swmatrix.h"

OS_STK UITask_stack[UI_TASK_STACK_SIZE];
static ui_cnf_t ui;

void led_display_update(void);

inline void keyboard_init() {
  PORTJ.DIRCLR = 0xF;
  PORTJ.OUTCLR = 0xF;
  PORTJ.PIN2CTRL = PORT_OPC_TOTEM_gc;
}
inline uint8_t keyboard_get() { return PORTJ.IN & 0xf; }
inline void led_display_oe_high() { PORTF.OUTSET = 0x10; }

inline void led_display_oe_low() { PORTF.OUTCLR = 0x10; }

inline void led_display_srclk_high() { PORTF.OUTSET = 0x80; }

inline void led_display_srclk_low() { PORTF.OUTCLR = 0x80; }

inline void led_display_data_high() { PORTF.OUTSET = 0x20; }

inline void led_display_data_low() { PORTF.OUTCLR = 0x20; }

inline void led_display_rclk_high() { PORTH.OUTSET = 0x80; }

inline void led_display_rclk_low() { PORTH.OUTCLR = 0x80; }

inline void led_display_init() {
  led_display_oe_high();
  PORTF.DIRSET = 0xF0;
  PORTH.DIRSET = 0x80;
}

inline void led_display_send8bits(uint8_t val) {
  led_display_rclk_low();
  uint8_t bit;
  for (bit = 0; bit < 8; bit++) {
    if (val & 0x80)
      led_display_data_high();
    else
      led_display_data_low();
    val <<= 1;
    led_display_srclk_low();
    led_display_srclk_high();
    led_display_srclk_low();
  }
  led_display_rclk_low();
  led_display_rclk_high();
  led_display_rclk_low();
}

inline void led_display_on() { led_display_oe_low(); }
inline void led_display_off() { led_display_oe_high(); }

void analog_supply_leds_enable(void);
void analog_supply_leds_disable(void);

void ui_set_display(ui_display_t displ) {
  ui.display = displ;
  led_display_update();
}

ui_display_t ui_get_display() { return ui.display; }

void ui_set_timeout(uint8_t timeout) { ui.timeout = timeout; }

#define SEGA 0x01
#define SEGB 0x02
#define SEGC 0x04
#define SEGD 0x08
#define SEGE 0x10
#define SEGF 0x20
#define SEGG 0x40
#define SEGH 0x80
uint8_t ui_is_display_active(void) {
  return !(ui.display == UI_DISPLAY_OFF ||
           (ui.display == UI_DISPLAY_AUTO && ui.autoDisplayState == 0));
}

uint8_t SEGMAP[] = {
    SEGA | SEGB | SEGC | SEGD | SEGE | SEGF,        // 0
    SEGB | SEGC,                                    // 1
    SEGA | SEGB | SEGD | SEGE | SEGG,               // 2
    SEGA | SEGB | SEGC | SEGD | SEGG,               // 3
    SEGB | SEGC | SEGF | SEGG,                      // 4
    SEGA | SEGC | SEGD | SEGF | SEGG,               // 5
    SEGA | SEGC | SEGD | SEGE | SEGF | SEGG,        // 6
    SEGA | SEGB | SEGC,                             // 7
    SEGA | SEGB | SEGC | SEGD | SEGE | SEGF | SEGG, // 8
    SEGA | SEGB | SEGC | SEGD | SEGF | SEGG,        // 9
    SEGA | SEGB | SEGC | SEGE | SEGF | SEGG,        // A
    SEGC | SEGD | SEGE | SEGF | SEGG,               // B
    SEGA | SEGD | SEGE | SEGF,                      // C
    SEGB | SEGC | SEGD | SEGE | SEGG,               // D
    SEGA | SEGD | SEGE | SEGF | SEGG,               // E
    SEGA | SEGE | SEGF | SEGG                       // F
};

void led_display_update(void) {
  uint16_t val = ui.value;
  ui_representation_t representation = ui.representation;
  uint8_t d0, d1, d2;
  if (val == 0xffff) {
    d0 = d1 = d2 = SEGF;
    led_display_send8bits(~SEGG);
    led_display_send8bits(~SEGG);
    led_display_send8bits(~SEGG);
  } else {
    if (representation == UI_REPRESENTATION_DEC) {
      d0 = val % 10;
      val /= 10;
      d1 = val % 10;
      val /= 10;
      d2 = val % 10;
    } else if (representation == UI_REPRESENTATION_HEX) {
      d0 = val & 0xf;
      val >>= 4;
      d1 = val & 0xf;
      val >>= 4;
      d2 = val & 0xf;
    } else // if(representation==UI_REPRESENTATION_OCT)
    {
      d0 = val & 0x7;
      val >>= 3;
      d1 = val & 0x7;
      val >>= 3;
      d2 = val & 0x7;
    }
    led_display_send8bits(~SEGMAP[d2]);
    led_display_send8bits(~SEGMAP[d1]);
    led_display_send8bits(~SEGMAP[d0]);
  }

  if (ui_is_display_active()) {
    led_display_on();
    analog_supply_leds_enable();
    LED_Off(LED_CV | LED_IV);
    if (swmatrix_get_meas() == SWMATRIX_MEAS_IV)
      LED_On(LED_IV);
    else
      LED_On(LED_CV);
  } else {
    led_display_off();
    analog_supply_leds_disable();
    LED_Off(LED_CV | LED_IV);
  }
}

void ui_set_representation(ui_representation_t representation) {
  ui.representation = representation;
  led_display_update();
}

void ui_set_value(uint16_t val) {
  ui.value = val;
  led_display_update();
  swmatrix_select_channel(val);
}

uint16_t ui_get_value() { return ui.value; }

ui_representation_t ui_get_representation() { return ui.representation; }

uint8_t ui_get_timeout() { return ui.timeout; }

void UiTask(void *pArg) {
  led_display_init();
  ui_set_representation(UI_REPRESENTATION_OCT);
  ui.timeElapsed = 0;
  ui.autoDisplayState = 1;
  keyboard_init();
  ui_set_value(0xffff);
  ui_set_timeout(10);
  ui_set_display(UI_DISPLAY_AUTO);
  led_display_update();
  //    uint8_t last_keyboard=keyboard_get();
  uint8_t keyboard;
  uint8_t pressed = 0;
  uint16_t newValue = 0;
  while (1) {
    OSTimeDlyHMSM(0, 0, 0, 10);
    //       LED_On(LED_HV);
    //       OSTimeDlyHMSM(0, 0, 0, 10);
    //       LED_Off(LED_HV);
    keyboard = keyboard_get();
    if (keyboard) {
      if (pressed > 10) {
        switch (keyboard) {
        case 0x01:
          newValue = ui_get_value() + 1;
          newValue &= 0xfff;
          ui_set_value(newValue);
          break;
        case 0x02:
          newValue = ui_get_value() - 1;
          newValue &= 0xfff;
          ui_set_value(newValue);
          break;
        case 0x03:
          newValue = 0xffff;
          ui_set_value(newValue);
          break;
        case 0x08:
          swmatrix_toggle_meas();
          break;
        }
        pressed = 0;
      }
      pressed++;
      led_display_update();
      ui.timeElapsed = 0;
      ui.autoDisplayState = 1;
    }
    if (keyboard == 0)
      pressed = 0;

    if (ui_get_display() == UI_DISPLAY_AUTO && ui.autoDisplayState) {
      ui.timeElapsed++;
      if (ui.timeElapsed / 100 >= ui.timeout) {
        ui.autoDisplayState = 0;
        led_display_update();
      }
    }
    //       last_keyboard=keyboard;
  }
}

DEFINE_COMMAND(ROOT_UI, INFO, NULL, pObj, args, pOut) {
  CLI_TPRINTF_ASSERT("UI. ");

  return RESULT_OK;
}

static IMMUTABLE_STR(DISPLAY_ON) = "ON";
static IMMUTABLE_STR(DISPLAY_OFF) = "OFF";
static IMMUTABLE_STR(DISPLAY_AUTO) = "AUTO";

static IMMUTABLE_STR(REPR_DEC) = "DEC";
static IMMUTABLE_STR(REPR_OCT) = "OCT";
static IMMUTABLE_STR(REPR_HEX) = "HEX";

DEFINE_COMMAND(ROOT_UI, DISPLAY, NULL, pObj, args, pOut) {
  args = skipSpaces(args);

  // get request
  if (strlen(args) == 0 || strcmp_P(args, S("?")) == 0) {
    if (ui_get_display() == UI_DISPLAY_AUTO)
      CLI_TPRINTFI_ASSERT(DISPLAY_AUTO);
    else if (ui_get_display() == UI_DISPLAY_ON)
      CLI_TPRINTFI_ASSERT(DISPLAY_ON);
    else
      CLI_TPRINTFI_ASSERT(DISPLAY_OFF);
    return RESULT_OK;
  }
  // set request
  else if (stricmp_P(args, DISPLAY_ON) == 0) {
    ui_set_display(UI_DISPLAY_ON);
    return RESULT_OK;
  } else if (stricmp_P(args, DISPLAY_OFF) == 0) {
    ui_set_display(UI_DISPLAY_OFF);
    return RESULT_OK;
  } else if (stricmp_P(args, DISPLAY_AUTO) == 0) {
    ui_set_display(UI_DISPLAY_AUTO);
    return RESULT_OK;
  } else
    return S("Expected OFF/ON/AUTO");

  return RESULT_OK;
}

DEFINE_COMMAND(ROOT_UI, TIMEOUT, NULL, pObj, args, pOut) {
  args = skipSpaces(args);
  if (strlen(args) == 0 || strcmp_P(args, S("?")) == 0) {
    CLI_TPRINTF_ASSERT("%d", ui_get_timeout());
  } else { // set dac
    int32_t val;
    result_t ret = parseInt(&args, 1, 60, &val);
    if (ret != RESULT_OK)
      return ret;
    ui_set_timeout(val);
  }
  return RESULT_OK;
}

DEFINE_COMMAND(ROOT_UI, DEBUG, NULL, pObj, args, pOut) {
  args = skipSpaces(args);
  if (strlen(args) == 0 || strcmp_P(args, S("?")) == 0) {
    CLI_TPRINTF_ASSERT("AutoDisplayState:%d\nTimeElapesed:%d\nKeyboard:%d",
                       ui.autoDisplayState, ui.timeElapsed, keyboard_get());
  }
  return RESULT_OK;
}

DEFINE_COMMAND(ROOT_UI, REPRESENTATION, NULL, pObj, args, pOut) {
  args = skipSpaces(args);

  // get request
  if (strlen(args) == 0 || strcmp_P(args, S("?")) == 0) {
    if (ui_get_representation() == UI_REPRESENTATION_DEC)
      CLI_TPRINTFI_ASSERT(REPR_DEC);
    else if (ui_get_representation() == UI_REPRESENTATION_HEX)
      CLI_TPRINTFI_ASSERT(REPR_HEX);
    else
      CLI_TPRINTFI_ASSERT(REPR_OCT);
    return RESULT_OK;
  }
  // set request
  else if (stricmp_P(args, REPR_DEC) == 0) {
    ui_set_representation(UI_REPRESENTATION_DEC);
    return RESULT_OK;
  } else if (stricmp_P(args, REPR_HEX) == 0) {
    ui_set_representation(UI_REPRESENTATION_HEX);
    return RESULT_OK;
  } else if (stricmp_P(args, REPR_OCT) == 0) {
    ui_set_representation(UI_REPRESENTATION_OCT);
    return RESULT_OK;
  } else
    return S("Expected DEC/HEX/OCT");
  return RESULT_OK;
}

DEFINE_COMMAND_ARRAY(ROOT, UI, INFO);
