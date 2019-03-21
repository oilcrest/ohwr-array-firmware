#ifndef _CONFIG_FILE_H__
#define _CONFIG_FILE_H__

#include "app_cfg.h" // for IDs of config sections
#include "types.h"

void ConfigFile_Init(void);

result_t ConfigFile_Load(uint16_t id, void *pData, size_t size);

result_t ConfigFile_Save(uint16_t id, const void *pData, size_t size);

#endif // !_CONFIG_FILE_H__
