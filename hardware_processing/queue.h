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
PUBLIC bool enqueue( uint16_t x );
PUBLIC bool dequeue( uint16_t *x );
PUBLIC bool set_queue_empty() ;
PUBLIC bool is_queue_full();
PUBLIC bool is_queue_empty(void);
PUBLIC uint16_t* return_buffer();
PUBLIC unsigned int return_front();
PUBLIC unsigned int return_rear();