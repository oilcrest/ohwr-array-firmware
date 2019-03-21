#include "config_file.h"

#include "eeprom_driver.h"
#include <string.h>

void ConfigFile_Init()
{
	EEPROM_EnableMapping();
	EEPROM_EnablePowerReduction();
	EEPROM_FlushBuffer();
}

result_t ConfigFile_Load(uint16_t id, void *pData, size_t size)
{
	memcpy(pData, (void *)(uintptr_t)(MAPPED_EEPROM_START + id), size);
	return RESULT_OK;
}

result_t ConfigFile_Save(uint16_t id, const void *pData, size_t size)
{
	uint8_t *p = (uint8_t *)pData;
	uint16_t page = id / EEPROM_PAGESIZE;
	while (size)
	{
		size_t k = size > EEPROM_PAGESIZE ? EEPROM_PAGESIZE : size;

		// fill buffer, erase and write.
		EEPROM_WaitForNVM();
		for (uint8_t i = 0; i < k; ++i)
		{
			EEPROM(page, i) = *p++;
		}
		EEPROM_ErasePage(page);
		EEPROM_SplitWritePage(page);
		EEPROM_WaitForNVM();

		size -= k;
		++page;
	}
	return RESULT_OK;
}

