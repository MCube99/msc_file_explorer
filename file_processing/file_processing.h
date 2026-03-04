// file_processing.h
#pragma once

#include "hardware_processing.h"

typedef enum
{
    MOUNT_SUCCESSFUL = 0,
    FILE_CREATED_SUCCESSFUL,
}FSUCCESS;

PUBLIC void file_processing_main();
PUBLIC void convert_ascii_to_string(uint8_t *buffer);