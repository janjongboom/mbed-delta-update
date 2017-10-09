/**
 * Some functions that are handy when implementing delta updates
 */

#ifndef _MBED_DELTA_UPDATE_H
#define _MBED_DELTA_UPDATE_H

#include "mbed.h"
#include "FlashIAP.h"

enum MBED_DELTA_UPDATE {
    MBED_DELTA_UPDATE_OK        = 0,
    MBED_DELTA_UPDATE_NO_MEMORY = -8401
};

int copy_flash_to_blockdevice(const uint32_t flash_page_size, size_t flash_address, size_t flash_size, BlockDevice *bd, size_t bd_address) {
    char *page_buffer = (char*)malloc(flash_page_size);
    if (!page_buffer) {
        return MBED_DELTA_UPDATE_NO_MEMORY;
    }

    int r;

    FlashIAP flash;
    if ((r = flash.init()) != 0) {
        free(page_buffer);
        return r;
    }

    int bytes_left = (int)flash_size;

    while (bytes_left > 0) {
        // copy it over
        int v = flash.read(page_buffer, flash_address, flash_page_size);
        if (v != 0) {
            free(page_buffer);
            return r;
        }
        bd->program(page_buffer, bd_address, flash_page_size);

        debug("Copying from flash to blockdevice: %d%%\n", ((flash_size - bytes_left) * 100) / flash_size);

        bytes_left -= flash_page_size;
        bd_address += flash_page_size;
        flash_address += flash_page_size;
    }

    free(page_buffer);

    if ((r = flash.deinit()) != 0) {
        return r;
    }

    debug("Copying from flash to blockdevice: 100%%\n");
    return MBED_DELTA_UPDATE_OK;
}

int print_blockdevice_content(BlockDevice *bd, size_t address, size_t length, size_t buffer_size) {
    uint8_t *buffer = (uint8_t*)malloc(buffer_size);
    if (!buffer) {
        return MBED_DELTA_UPDATE_NO_MEMORY;
    }

    size_t offset = address;
    size_t bytes_left = length;

    while (bytes_left > 0) {
        size_t length = buffer_size;
        if (length > bytes_left) length = bytes_left;

        bd->read(buffer, offset, length);

        for (size_t ix = 0; ix < length; ix++) {
            debug("%02x", buffer[ix]);
        }

        offset += length;
        bytes_left -= length;
    }

    debug("\n");

    free(buffer);

    return MBED_DELTA_UPDATE_OK;
}

#endif // _MBED_DELTA_UPDATE_H
