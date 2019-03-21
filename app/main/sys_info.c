/**
 *  \file
 *
 *  \brief System strings
 *
 *  \author Adrian Matoga, AGH-UST Cracow
 *  \author Szymon Kulis, AGH-UST Cracow
 */

#include "sys_info.h"

IMMUTABLE_STR(SysInfo_banner) = "Switching matrix, CERN";

IMMUTABLE_STR(SysInfo_build_rev) = "Git tag " SYS_INFO_BUILD_REVISION;

IMMUTABLE_STR(SysInfo_build_date) = "Build date " SYS_INFO_BUILD_DATE;

IMMUTABLE_STR(SysInfo_build_user) = "Build user " SYS_INFO_BUILD_USER;

IMMUTABLE_STR(SysInfo_build_machine) = "Build machine " SYS_INFO_BUILD_MACHINE;

IMMUTABLE_STR(SysInfo_cpu) = "AVR ATxmega128a1";
