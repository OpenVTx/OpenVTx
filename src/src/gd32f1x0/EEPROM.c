#include "EEPROM.h"
#include <gd32f1x0.h>
#include <string.h>

#ifndef FLASH_BASE
#define FLASH_BASE  0x08000000U
#endif
#define FLASH_SIZE  0x8000U     // 32kB
#define FLASH_END   (FLASH_BASE + FLASH_SIZE)
#define PAGE_SIZE   0x400U

#define EEPROM_SIZE (PAGE_SIZE)
#define EEPROM_ADDR (FLASH_END - EEPROM_SIZE)


static int flash_storage_erase(uint32_t address)
{
    /* unlock the flash program/erase controller */
    //fmc_unlock();

    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR);

    /* erase the flash pages */
    for (; address < FLASH_END; address += PAGE_SIZE) {
        if (fmc_page_erase(address) != FMC_READY) {
            break;
        }
        fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR);
    }

    /* lock the main FMC after the erase operation */
    //fmc_lock();

    return (address >= FLASH_END) ? 0 : -1;
}


static int flash_storage_write(uint32_t const * data, uint32_t size)
{
    uint32_t address = EEPROM_ADDR;

    if (!data || !size || EEPROM_SIZE < size)
        return -1;

    if (!memcmp((void*)address, data, size))
        return -1;

    /* unlock the flash program/erase controller */
    fmc_unlock();

    if (flash_storage_erase(address) < 0) {
        // Erase failed
        fmc_lock();
        return -1;
    }

    /* program flash */
    while (size-- && address < FLASH_END) {
        if (fmc_word_program(address, *data++) != FMC_READY) {
            break;
        }
        address += 4U;
        fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_PGERR);
    }

    /* lock the main FMC after the program operation */
    fmc_lock();

    if (size)
        return -1;

    return 0;
}


void eeprom_update_block(const uint16_t idx, uint8_t *ptr, uint32_t const len)
{
    (void)idx;
    fwdgt_counter_reload();
    flash_storage_write((uint32_t*)ptr, ((len + 3) / 4));
    fwdgt_counter_reload();
}


void eeprom_read_block(const uint16_t idx, uint8_t *ptr, uint32_t const len)
{
    (void)idx;
    if (!ptr || !len || EEPROM_SIZE < len)
        return;
    memcpy(ptr, (uint8_t*)EEPROM_ADDR, len);
}
