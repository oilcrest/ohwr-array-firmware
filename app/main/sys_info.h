/**
 *  \file
 *
 *  \brief System info.
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 */

#ifndef _SYS_INFO_H__
#define _SYS_INFO_H__

#include "types.h"

extern IMMUTABLE_STR(SysInfo_banner);

extern IMMUTABLE_STR(SysInfo_build_rev);

extern IMMUTABLE_STR(SysInfo_build_date);

extern IMMUTABLE_STR(SysInfo_build_user);

extern IMMUTABLE_STR(SysInfo_build_machine);

extern IMMUTABLE_STR(SysInfo_cpu);

extern IMMUTABLE_STR(SysInfo_fpga);

static inline immutable_str SysInfo_GetCpuName(void) { return SysInfo_cpu; }

#endif // !_SYS_INFO_H__
