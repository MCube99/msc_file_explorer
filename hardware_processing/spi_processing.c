// Copyright (c) 2021 Michael Stoops. All rights reserved.
// Portions copyright (c) 2021 Raspberry Pi (Trading) Ltd.
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
// following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
//    disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
//    following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
//    products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// SPDX-License-Identifier: BSD-3-Clause
//
// Example of an SPI bus slave using the PL022 SPI interface

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware_processing.h"




#define spi_default                         spi0
#define PICO_DEFAULT_SPI_SCK_PIN              2 
#define PICO_DEFAULT_SPI_RX_PIN               4  //mosi     //  PICO_DEFAULT_SPI_TX_PIN  
#define PICO_DEFAULT_SPI_TX_PIN               3  //miso     #define PICO_DEFAULT_SPI_RX_PIN 
#define PICO_DEFAULT_SPI_CSN_PIN              5  // chip select
#define SPI_IRQ_PIN                           6


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


#define RXMIS_BIT                           RXIM_BIT // RX Masked Interrupt Status bit in SSPMIS register (basically data is ready to be read)
#define RORMIS_BIT                          RORIM_BIT  // ROR Masked Interrupt Status bit in SSPMIS register (basically overrun error has occurred)


volatile DWORD *spi_base       = ( volatile DWORD * )SPI0_BASE;
volatile DWORD *spi_data_reg  = ( volatile DWORD * )( SPI0_BASE + SPI_DATA_REGISTER_OFFSET );  
volatile DWORD *spi_status_reg = ( volatile DWORD * )( SPI0_BASE + SPI_STATUS_OFFSET );
volatile DWORD *spi_interrupt_mask = ( volatile DWORD * )( SPI0_BASE + SPI_INTERRUPT_MASK_SET_OFFSET );
volatile DWORD *spi_interrupt_mask_clear = ( volatile DWORD * )( SPI0_BASE + SPI_INTERRUPT_MASK_CLEAR_OFFSET );
volatile DWORD *spi_interrupt_status = ( volatile DWORD * )( SPI0_BASE + SPI_INTERRUPT_MASK_STATUS_OFFSET ); 
volatile DWORD *spi_interrupt_clear = ( volatile DWORD * )( SPI0_BASE + SPI_INTERRUPT_CLEAR_OFFSET );




PRIVATE void convert_hex_to_string(const uint8_t *byte_array, int length, char *conversion);



PUBLIC void prepare_memory_for_spi_transfer(char *conversion, int size, uint8_t *out_buf) {

    // Placeholder for any memory preparation needed before SPI transfer

    memset(conversion, 0 , size + 1); // Initialize conversion buffer);
    memset(out_buf, 0 , size); // Initialize output buffer
}

PUBLIC void set_spi_gpio_pins() {

    spi_init(spi_default,  1000 * 1000); // 1 MHz

    spi_set_format(
    spi0,
    8,                    // bits
    SPI_CPOL_0,
    SPI_CPHA_0,
    SPI_MSB_FIRST
    );

    spi_set_slave(spi_default, true);
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_IRQ_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(SPI_IRQ_PIN, GPIO_IN);
    gpio_set_input_enabled(SPI_IRQ_PIN, true); // interrupt line needs to detect input


}

PUBLIC bool spi_irq_setup_init(void) {
    
    // Mask the appropriate interrupts

    spi0_hw->imsc |= RXIM_BIT; // RXIM interrupt enable
    spi0_hw->imsc |= RORIM_BIT; // For overrun errors

    // Set the IRQ handler
    irq_set_exclusive_handler(SPI0_IRQ, hardware_processing);

    // Enable SPI0 IRQ
    irq_set_enabled(SPI0_IRQ, true);

    // Enable GPIO IRQs for the CSN pin
    gpio_set_irq_enabled_with_callback(SPI_IRQ_PIN,
                         GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
                         true, &hardware_processing ); // use  

    // Return whether the SPI0 IRQ is enabled
    return irq_is_enabled(SPI0_IRQ);
}


PUBLIC void hardware_processing( uint8_t *in_buf, char *conversion) {
     for (size_t i = 0; i < BUF_LEN; ++i) {
          spi_read_blocking( spi_default, 0x00, in_buf, NUMBER_OF_BYTES ); // read one byte at a time
          sleep_ms(10 * 1000); // wait for 10 seconds before next read

     }
     
  

    convert_hex_to_string(in_buf, NUMBER_OF_BYTES, conversion);
}



PRIVATE void convert_hex_to_string(const uint8_t *byte_array, int length, char *conversion)
{
    uint8_t x = 0;

    while (x < length)
    {
        conversion[x] = (char)byte_array[x];
        x++;
    }
    
    conversion[x] = '\0';   
}





