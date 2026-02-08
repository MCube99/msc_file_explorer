#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "hardware_processing.h"


#define PRIVATE static
#define PUBLIC  extern  

#define BUF_LEN                 0x100  // 256 bytes
#define NUMBER_OF_BYTES         BUF_LEN 

typedef uint8_t  BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

PUBLIC bool spi_irq_setup_init( void );
PUBLIC void prepare_memory_for_spi_transfer( uint8_t *in_buf ); 
PUBLIC void set_spi_gpio_pins( void );
PUBLIC void irq_processing_main( uint8_t *in_buf );




#define SPI0_BASE                            0x4003C000
#define SPI_DATA_REGISTER_OFFSET             0x08 // Offset for the SPI Data Register SSPDR
#define SPI_STATUS_OFFSET                    0x0C // Offset for the SPI Status Register SSPSR
#define SPI_INTERRUPT_MASK_SET_OFFSET        0x014 // Offset for the SPI Mask Set Register SSPIMSC
#define SPI_INTERRUPT_MASK_CLEAR_OFFSET      0x018 // Offset for the SPI Mask Clear Register SSPIMSC
#define SPI_INTERRUPT_MASK_STATUS_OFFSET     0x01c // Offset for the SPI Mask Status Register SSPMIS
#define SPI_INTERRUPT_CLEAR_OFFSET           0x020 // Offset for the SPI Interrupt Clear Register SSPICR

#define RXIM_BIT                            ( 1U << 2 ) // RX Interrupt Mask bit in SSPIMSC register
#define RORIM_BIT                           ( 1U << 0 ) // ROR Interrupt Mask bit in SSPIMSC register

#define RTIC_BIT                            ( 1U << 1 ) // RT Interrupt Clear bit in SSPICR register
#define RORIC_BIT                           ( 1U << 0 ) // ROR Interrupt Clear bit in SSPICR register

#define RX_NOT_EMPTY_BIT                    ( 1U << 2 ) // RX FIFO Not Empty bit in SSPSR register
#define RX_FULL_BIT                         ( 1U << 3 ) // RX FIFO Full bit in SSPSR register


#define RXMIS_BIT                           RXIM_BIT // RX Masked Interrupt Status bit in SSPMIS register (basically data is ready to be read)
#define RORMIS_BIT                          RORIM_BIT  // ROR Masked Interrupt Status bit in SSPMIS register (basically overrun error has occurred)


PUBLIC volatile DWORD *spi_base_reg;      
PUBLIC volatile DWORD *spi_data_reg;
PUBLIC volatile DWORD *spi_status_reg;
PUBLIC volatile DWORD *spi_interrupt_mask;
PUBLIC volatile DWORD *spi_interrupt_mask_clear;
PUBLIC volatile DWORD *spi_interrupt_status; 
PUBLIC volatile DWORD *spi_interrupt_clear;


extern volatile bool spi_interrupt;
extern volatile uint16_t spi_rx_len; 

extern volatile bool spi_reading;
