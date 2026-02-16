#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware_processing.h"


#define PRIVATE static
#define PUBLIC  extern  

#define BUF_LEN                 0x100  // 256 bytes
#define NUMBER_OF_BYTES         BUF_LEN 



typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

PUBLIC bool spi_irq_setup_init( void );
PUBLIC void set_spi_gpio_pins( void );
PUBLIC void irq_processing_main( uint8_t *in_buf );

PUBLIC void prepare_memory_for_spi_transfer( uint8_t *buffer, uint8_t *copy_buffer );
PUBLIC void copy_queue_buffer( void );
PUBLIC void append_char(uint16_t byte);
PUBLIC bool spi_is_processing();
PUBLIC void gpio_set_irq_active(uint gpio, uint32_t events, bool enabled);

 




extern volatile bool csn_low;

// extern volatile uint16_t spi_rx_len; 

// typedef struct Buffers *buffers;


