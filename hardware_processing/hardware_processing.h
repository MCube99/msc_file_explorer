#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "pico/stdlib.h"


#define PRIVATE static
#define PUBLIC  extern  

#define BUF_LEN                 0x100  // 256 bytes
#define NUMBER_OF_BYTES         BUF_LEN 

typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

PUBLIC bool spi_irq_setup_init( void );
PUBLIC void prepare_memory_for_spi_transfer( uint8_t *in_buf ); 
PUBLIC void set_gpio_pins() ;
PUBLIC void set_spi_gpio_pins( void );
PUBLIC void gpio_set_irq_active(uint gpio, uint32_t events, bool enabled);
PUBLIC void spi0_irq_handler();
 PUBLIC void copy_queue_buffer( void );
PUBLIC bool spi_is_processing();

extern volatile bool spi_reading;
extern volatile bool csn_high;
