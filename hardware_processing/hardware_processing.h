#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PRIVATE static
#define PUBLIC  extern  

#define BUF_LEN 0x100  // 256 bytes
#define NUMBER_OF_BYTES BUF_LEN 

typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

PUBLIC bool spi_irq_setup_init(void);
PUBLIC void prepare_memory_for_spi_transfer( char *conversion, int size, uint8_t *out_buf );
PUBLIC void set_spi_gpio_pins(void);
PUBLIC void hardware_processing(   uint8_t *in_buf, char *conversion ); 

extern volatile bool spi_transfer;