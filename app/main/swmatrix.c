/**
 *  \file
 *
 *  \brief Switching matrix driver
 *
 *  \author Szymon Kulis, CERN, Geneva
 */

#include "swmatrix.h"
#include "TWI_master.h"
#include "astring.h"
#include "cli.h"
#include "cmdarg.h"
#include "debug.h"

void swmatrix_switches_all_shorted();
void led_display_update(void);
void ui_set_value(uint16_t val);
uint16_t ui_get_value(void);

swmatrix_mode_t mode;
swmatrix_meas_t meas;
swmatrix_cvres_t cvres;

static IMMUTABLE_STR(iv) = "IV";
static IMMUTABLE_STR(cv) = "CV";
static IMMUTABLE_STR(CVRES_100K) = "100K";
static IMMUTABLE_STR(CVRES_500K) = "500K";
static IMMUTABLE_STR(CVRES_1M) = "1M";
static IMMUTABLE_STR(CVRES_2M) = "2M";
static IMMUTABLE_STR(CVRES_5M) = "5M";
static IMMUTABLE_STR(CVRES_10M) = "10M";
static IMMUTABLE_STR(CVRES_50M) = "50M";
static IMMUTABLE_STR(CVRES_100M) = "100M";

static twi_iface_t i2cMatrix = {0x80, 0x40};
static twi_iface_t i2cProbeCard = {0x08, 0x04};

result_t swmatrix_init(void) {
  OSTimeDlyHMSM(0, 0, 0, 200);
  // A0-A8 + switches
  PORTC.OUTCLR = 0xff;
  PORTD.OUTCLR = 0x01;

  PORTC.DIRSET = 0xff;
  PORTD.DIRSET = 0xFF;
  // mux enable
  PORTE.OUTSET = 0x04;
  PORTE.DIRSET = 0x04;
  // mux enable (EN_A)
  PORTB.OUTSET = 0x08;
  PORTB.DIRSET = 0x08;
  // MT0. MT1
  PORTE.DIRSET = 0x03;

  // CVM2 - CVM0
  PORTH.DIRSET = 0x07;

  // sync 0
  PORTD.OUTCLR = 0x10;

  // switches reset
  PORTD.OUTSET = 0x8; // reset is active (signal in inverted on the board)
  PORTD.OUTSET = 0x8; // reset is active (signal in inverted on the board)
  PORTD.OUTSET = 0x8; // reset is active (signal in inverted on the board)
  volatile uint8_t i;
  for (i = 0; i < 10; i++)
    ;
  PORTD.OUTCLR = 0x8; // reset is inactive (signal in inverted on the board)
  swmatrix_switches_all_shorted();
  swmatrix_set_meas(SWMATRIX_MEAS_CV);
  return RESULT_OK;
}

void swmatrix_set_meas(swmatrix_meas_t _meas) {
  meas = _meas;
  if (meas == SWMATRIX_MEAS_CV) {
    PORTE.OUTSET = 0x02;
    PORTE.OUTCLR = 0x01;
  } else {
    PORTE.OUTSET = 0x01;
    PORTE.OUTCLR = 0x02;
  }
}

swmatrix_meas_t swmatrix_get_meas() { return meas; }

void swmatrix_toggle_meas() {
  if (meas == SWMATRIX_MEAS_CV)
    swmatrix_set_meas(SWMATRIX_MEAS_IV);
  else
    swmatrix_set_meas(SWMATRIX_MEAS_CV);
}

void swmatrix_switches_send_byte(uint8_t b) {
  for (uint8_t bit = 0; bit < 8; bit++) {
    // there is an inverter on the board
    if (b & 0x80)
      PORTD.OUTCLR = 0x20;
    else
      PORTD.OUTSET = 0x20;
    volatile uint8_t i;
    for (i = 0; i < 30; i++)
      ;
    // clock 1
    PORTD.OUTSET = 0x80;
    PORTD.OUTSET = 0x80;
    for (i = 0; i < 30; i++)
      ;
    b <<= 1;
    // clock 0
    PORTD.OUTCLR = 0x80;
    for (i = 0; i < 30; i++)
      ;
  }
}

void swmatrix_switches_all_shorted() {
  // clock 1
  PORTD.OUTSET = 0x80;
  PORTD.OUTSET = 0x80;
  // sync 1
  PORTD.OUTSET = 0x10;
  // clock 0
  PORTD.OUTCLR = 0x80;
  PORTD.OUTCLR = 0x80;
  PORTD.OUTCLR = 0x80;
  PORTD.OUTCLR = 0x80;

  // sync 0
  PORTD.OUTCLR = 0x10;

  for (uint8_t chip = 0; chip < 64; chip++)
    swmatrix_switches_send_byte(0xFF);
  // sync 1
  PORTD.OUTSET = 0x10;
}

void swmatrix_switches_open_one_channel(uint16_t chn) {
  // clock 1
  PORTD.OUTSET = 0x80;
  PORTD.OUTSET = 0x80;
  // sync 1
  PORTD.OUTSET = 0x10;
  // clock 0
  PORTD.OUTCLR = 0x80;
  PORTD.OUTCLR = 0x80;
  PORTD.OUTCLR = 0x80;
  PORTD.OUTCLR = 0x80;

  // sync 0
  PORTD.OUTCLR = 0x10;

  uint8_t chnInChip = chn % 8;
  uint8_t chipInRow = (chn / 8) % 8;
  uint8_t row = (chn / 64) % 8;
  uint8_t mapping[] = {3, 2, 1, 0, 7, 6, 5, 4};

  for (int8_t r = 7; r >= 0; r--)
    for (int8_t c = 0; c < 8; c++) // chip
    {
      uint8_t byte = 0xff;
      if (row == r && c == chipInRow)
        byte = ~(1 << (7 - mapping[chnInChip]));
      swmatrix_switches_send_byte(byte);
    }
  // sync 1
  PORTD.OUTSET = 0x10;
}

void swmatrix_select_channel(uint16_t chn) {
  uint16_t chnraw = chn;
  swmatrix_switches_all_shorted();
  if (chn != 0xffff) {
    uint8_t a6a7a8 = (chn >> 6) & 0x7;
    chn &= ~(0x100 | 0x080 | 0x040);
    uint8_t a6a7a8_map[] = {4, 5, 6, 7, 3, 2, 1, 0};
    uint8_t a6a7a8_mapped = a6a7a8_map[a6a7a8];
    chn |= ((uint16_t)a6a7a8_mapped) << 6;

    uint8_t a0a1a2 = (chn)&0x7;
    chn &= ~(0x1 | 0x02 | 0x4);
    uint8_t a0a1a2_map[] = {3, 2, 1, 0, 4, 5, 6, 7};
    uint8_t a0a1a2_mapped = a0a1a2_map[a0a1a2];
    chn |= ((uint16_t)a0a1a2_mapped);

    PORTC.OUT = chn & 0xff;
    if (chn & 0x100)
      PORTD.OUTSET = 0x01;
    else
      PORTD.OUTCLR = 0x01;
    swmatrix_switches_open_one_channel(chnraw);
  }
}

swmatrix_mode_t swmatrix_get_mode() { return mode; }

void swmatrix_set_mode(swmatrix_mode_t _mode) { mode = _mode; }

swmatrix_cvres_t swmatrix_get_cvres(void) { return cvres; }

void swmatrix_set_cvres(swmatrix_cvres_t cvres_) {
  cvres = cvres_;
  PORTH.OUTCLR = 0x07;
  PORTH.OUTSET = cvres & 0x07;
}

//******************************************************************************
// user interface
//******************************************************************************

DEFINE_COMMAND(ROOT_MATRIX, SHORTALL, NULL, pObj, args, pOut) {
  swmatrix_switches_all_shorted();
  ui_set_value(0xffff);
  return RESULT_OK;
}

DEFINE_COMMAND(ROOT_MATRIX, CHANNEL, NULL, pObj, args, pOut) {
  args = skipSpaces(args);
  if (strlen(args) == 0 || strcmp_P(args, S("?")) == 0) {
    if (ui_get_value() == 0xFFFF)
      CLI_TPRINTF_ASSERT("---");
    else {
      int16_t val = ui_get_value();
      val = val & 0xfff;
      CLI_TPRINTF_ASSERT("%d", val);
    }
  } else { // set dac
    int32_t val;
    result_t ret = parseInt(&args, 0, 511, &val);
    if (ret != RESULT_OK)
      return ret;
    ui_set_value(val);
  }
  return RESULT_OK;
}

DEFINE_COMMAND(ROOT_MATRIX, MEASUREMENT, NULL, pObj, args, pOut) {
  args = skipSpaces(args);
  // get request
  if (strlen(args) == 0 || strcmp_P(args, S("?")) == 0) {
    if (swmatrix_get_meas() == SWMATRIX_MEAS_CV)
      CLI_TPRINTFI_ASSERT(cv);
    else
      CLI_TPRINTFI_ASSERT(iv);
    return RESULT_OK;
  }
  // set request
  else if (stricmp_P(args, iv) == 0) {
    swmatrix_set_meas(SWMATRIX_MEAS_IV);
    led_display_update();
    return RESULT_OK;
  } else if (stricmp_P(args, cv) == 0) {
    swmatrix_set_meas(SWMATRIX_MEAS_CV);
    led_display_update();
    return RESULT_OK;
  } else
    return S("Expected IV or CV");
}

DEFINE_COMMAND(ROOT_MATRIX, CVRES, NULL, pObj, args, pOut) {
  args = skipSpaces(args);
  // get request
  if (strlen(args) == 0 || strcmp_P(args, S("?")) == 0) {
    swmatrix_cvres_t res = swmatrix_get_cvres();
    switch (res) {
    case SWMATRIX_CVRES_100K:
      CLI_TPRINTFI_ASSERT(CVRES_100K);
      break;
    case SWMATRIX_CVRES_500K:
      CLI_TPRINTFI_ASSERT(CVRES_500K);
      break;
    case SWMATRIX_CVRES_1M:
      CLI_TPRINTFI_ASSERT(CVRES_1M);
      break;
    case SWMATRIX_CVRES_2M:
      CLI_TPRINTFI_ASSERT(CVRES_2M);
      break;
    case SWMATRIX_CVRES_5M:
      CLI_TPRINTFI_ASSERT(CVRES_5M);
      break;
    case SWMATRIX_CVRES_10M:
      CLI_TPRINTFI_ASSERT(CVRES_10M);
      break;
    case SWMATRIX_CVRES_50M:
      CLI_TPRINTFI_ASSERT(CVRES_50M);
      break;
    case SWMATRIX_CVRES_100M:
      CLI_TPRINTFI_ASSERT(CVRES_100M);
      break;
    }
    return RESULT_OK;
  }
  // set request
  else if (stricmp_P(args, CVRES_100K) == 0) {
    swmatrix_set_cvres(SWMATRIX_CVRES_100K);
    return RESULT_OK;
  } else if (stricmp_P(args, CVRES_500K) == 0) {
    swmatrix_set_cvres(SWMATRIX_CVRES_500K);
    return RESULT_OK;
  } else if (stricmp_P(args, CVRES_1M) == 0) {
    swmatrix_set_cvres(SWMATRIX_CVRES_1M);
    return RESULT_OK;
  } else if (stricmp_P(args, CVRES_2M) == 0) {
    swmatrix_set_cvres(SWMATRIX_CVRES_2M);
    return RESULT_OK;
  } else if (stricmp_P(args, CVRES_5M) == 0) {
    swmatrix_set_cvres(SWMATRIX_CVRES_5M);
    return RESULT_OK;
  } else if (stricmp_P(args, CVRES_10M) == 0) {
    swmatrix_set_cvres(SWMATRIX_CVRES_10M);
    return RESULT_OK;
  } else if (stricmp_P(args, CVRES_50M) == 0) {
    swmatrix_set_cvres(SWMATRIX_CVRES_50M);
    return RESULT_OK;
  } else if (stricmp_P(args, CVRES_100M) == 0) {
    swmatrix_set_cvres(SWMATRIX_CVRES_100M);
    return RESULT_OK;
  } else
    return S("Expected 100K, 500K, 1M, 2M, 5M, 10M, 50M, 100M");
}

DEFINE_COMMAND(ROOT_MATRIX, INFO, NULL, pObj, args, pOut) {
  CLI_TPRINTF_ASSERT("Switching MATRIX related commands.");

  return RESULT_OK;
}

result_t getI2Ctemp(twi_iface_t *iface, void *pOut) {
  unsigned char data[4];
  char err = 0;
  twi_init(iface);
  data[0] = 0xe3; // Trigger T measurement hold master
  err = twi_write_data(iface, 0x40, data, 1);
  err = twi_read_data(iface, 0x40, data, 3);
  uint16_t st = data[0];
  st <<= 8;
  st |= data[1];
  float temp = -46.85 + 0.0026812744140625 * st;
  if (err)
    return CLI_TPRINTF("Error");
  return CLI_TPRINTF("%.4f", temp);
}

result_t getI2Chumidity(twi_iface_t *iface, void *pOut) {
  unsigned char data[4];
  char err = 0;
  twi_init(iface);
  data[0] = 0xe5; // Trigger T measurement hold master
  err = twi_write_data(iface, 0x40, data, 1);
  err = twi_read_data(iface, 0x40, data, 3);
  uint16_t rh = data[0];
  rh <<= 8;
  rh |= data[1];
  float relhum = -6 + 0.0019073486328125 * rh;
  if (err)
    return CLI_TPRINTF("Error");
  return CLI_TPRINTF("%.4f", relhum);
}

DEFINE_COMMAND(ROOT_MATRIX, TEMPERATURE, NULL, pObj, args, pOut) {
  return getI2Ctemp(&i2cMatrix, pOut);
}

DEFINE_COMMAND(ROOT_MATRIX, HUMIDITY, NULL, pObj, args, pOut) {
  return getI2Chumidity(&i2cMatrix, pOut);
}

DEFINE_COMMAND(ROOT_PROBECARD, TEMPERATURE, NULL, pObj, args, pOut) {
  return getI2Ctemp(&i2cProbeCard, pOut);
}

DEFINE_COMMAND(ROOT_PROBECARD, HUMIDITY, NULL, pObj, args, pOut) {
  return getI2Chumidity(&i2cProbeCard, pOut);
}
DEFINE_COMMAND(ROOT_PROBECARD, INFO, NULL, pObj, args, pOut) {
  CLI_TPRINTF_ASSERT("Probecard related commands.");

  return RESULT_OK;
}

DEFINE_COMMAND_ARRAY(ROOT, PROBECARD, INFO);

DEFINE_COMMAND_ARRAY(ROOT, MATRIX, INFO);
