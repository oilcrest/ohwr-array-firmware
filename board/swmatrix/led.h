/**
 *  \file
 *
 *  \brief Test beam 2 readout board LEDs driver.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _LED_H__
#define _LED_H__

#define LED_HV 0x40
#define LED_STATUS 0x20
#define LED_CV 0x10
#define LED_IV 0x08

/// All four LEDs mask
#define LED_ALL (LED_HV | LED_STATUS | LED_CV | LED_IV)

/** \brief Setup LED GPIO pins.
 */
#define LED_Init()                                                             \
  do {                                                                         \
    PORTH.DIRSET = LED_ALL;                                                    \
  } while (0)

/** \brief Switch on LED(s).
 *
 *  \param[in]  _mask Mask of LEDs to switch on.
 */
#define LED_On(_mask)                                                          \
  do {                                                                         \
    PORTH.OUTSET = (uint8_t)(_mask);                                           \
  } while (0)

/** \brief Switch off LED(s).
 *
 *  \param[in]  _mask Mask of LEDs to switch off.
 */
#define LED_Off(_mask)                                                         \
  do {                                                                         \
    PORTH.OUTCLR = (uint8_t)(_mask);                                           \
  } while (0)

/** \brief Toggle LED(s).
 *
 *  \param[in]  _mask Mask of LEDs to toggle.
 */
#define LED_Toggle(_mask)                                                      \
  do {                                                                         \
    PORTH.OUTTGL = (uint8_t)(_mask);                                           \
  } while (0);

#endif // !_LED_H__
