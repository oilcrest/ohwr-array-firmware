#ifndef __SWMATRIX_H__
#define __SWMATRIX_H__
#include "command.h"
#include "stream.h"
#include "terminal.h"
#include "types.h"
#include "ucos_ii.h"

typedef enum swmatrix_mode_enum {
  SWMATRIX_MODE_GND = 0,
  SWMATRIX_MODE_OPEN = 1
} swmatrix_mode_t;

typedef enum swmatrix_meas_enum {
  SWMATRIX_MEAS_IV = 0,
  SWMATRIX_MEAS_CV = 1
} swmatrix_meas_t;

/* this is how it should be
typedef enum swmatrix_cvres
{
 SWMATRIX_CVRES_100K=0,
 SWMATRIX_CVRES_500K=1,
 SWMATRIX_CVRES_1M=2,
 SWMATRIX_CVRES_2M=3,
 SWMATRIX_CVRES_5M=4,
 SWMATRIX_CVRES_10M=5,
 SWMATRIX_CVRES_50M=6,
 SWMATRIX_CVRES_100M=7
}
swmatrix_cvres_t;
*/

// it looks like CVM[2:0] are swaped to CVM[0:2]
typedef enum swmatrix_cvres {
  SWMATRIX_CVRES_100K = 0,
  SWMATRIX_CVRES_500K = 4,
  SWMATRIX_CVRES_1M = 2,
  SWMATRIX_CVRES_2M = 4 | 2,
  SWMATRIX_CVRES_5M = 1,
  SWMATRIX_CVRES_10M = 4 | 1,
  SWMATRIX_CVRES_50M = 2 | 1,
  SWMATRIX_CVRES_100M = 4 | 2 | 1
} swmatrix_cvres_t;

result_t swmatrix_init(void);
swmatrix_mode_t swmatrix_get_mode(void);
void swmatrix_set_mode(swmatrix_mode_t mode);

swmatrix_meas_t swmatrix_get_meas(void);
void swmatrix_set_meas(swmatrix_meas_t meas);
void swmatrix_toggle_meas(void);

void swmatrix_select_channel(uint16_t chn);

swmatrix_cvres_t swmatrix_get_cvres(void);
void swmatrix_set_cvres(swmatrix_cvres_t cvres);

#endif // !__SWMATRIX_H__
