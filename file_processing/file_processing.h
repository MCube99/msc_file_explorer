// file_processing.h
#pragma once

#include "hardware_processing.h"

typedef enum
{
    MOUNT_SUCCESSFUL = 0,
    FILE_CREATED_SUCCESSFUL,
}FSUCCESS;

PUBLIC void file_processing_main();
PUBLIC unsigned int append_char(uint32_t byte);