#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "hardware_processing.h"

// Flags to indicate SPI status


// struct queue_type {
//     int front;          // read index
//     int rear;           // write index
//     int buffer[ BUF_LEN ];
//     SPI_STATE spi_state; // SPI status flag
// }; 

// struct queue_type;
// // Opaque pointer to queue structure
// typedef struct queue_type * Queue;

// ADT API
PUBLIC void queue_init();
PUBLIC bool enqueue( uint32_t x );
PUBLIC bool dequeue(uint32_t *x );
PUBLIC bool set_queue_empty() ;
PUBLIC bool queue_is_full();
PUBLIC bool queue_is_empty(void);
PUBLIC int GetNextByte();
PUBLIC int GetBytesAvailable();