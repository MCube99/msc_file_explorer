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
#include "queue.h"
#include "hardware/sync.h"
#include "pico/time.h"


#define spi_default                         spi0
// #define PICO_DEFAULT_SPI_SCK_PIN            2   // SPI clock, same as master
// #define PICO_DEFAULT_SPI_RX_PIN             4
// #define PICO_DEFAULT_SPI_TX_PIN             3
// #define PICO_DEFAULT_SPI_CSN_PIN            5    

#define PICO_DEFAULT_SPI_SCK_PIN            2   // SPI clock, same as master
#define PICO_DEFAULT_SPI_RX_PIN             0   // MOSI from master → receive on slave
#define PICO_DEFAULT_SPI_TX_PIN             3   // MISO to master → send from slave
#define PICO_DEFAULT_SPI_CSN_PIN            1   // Chip select
#define SPI_IRQ_PIN                         6   // GPIO pin for SPI interrupt line from master


volatile DWORD *spi_base_reg       = ( volatile DWORD * )SPI0_BASE;
volatile DWORD *spi_data_reg  = ( volatile DWORD * )( SPI0_BASE + SPI_DATA_REGISTER_OFFSET );  
volatile DWORD *spi_status_reg = ( volatile DWORD * )( SPI0_BASE + SPI_STATUS_OFFSET );
volatile DWORD *spi_interrupt_mask = ( volatile DWORD * )( SPI0_BASE + SPI_INTERRUPT_MASK_SET_OFFSET );
volatile DWORD *spi_interrupt_mask_clear = ( volatile DWORD * )( SPI0_BASE + SPI_INTERRUPT_MASK_CLEAR_OFFSET );
volatile DWORD *spi_interrupt_status = ( volatile DWORD * )( SPI0_BASE + SPI_INTERRUPT_MASK_STATUS_OFFSET ); 
volatile DWORD *spi_interrupt_clear = ( volatile DWORD * )( SPI0_BASE + SPI_INTERRUPT_CLEAR_OFFSET );


PRIVATE void convert_hex_to_string(const uint8_t *byte_array, int length, char *conversion);
PRIVATE void irq_handler(void);






PUBLIC void prepare_memory_for_spi_transfer(uint8_t *in_buf) {

    // Placeholder for any memory preparation needed before SPI transfer

    
    memset(in_buf, 0 , NUMBER_OF_BYTES); // Initialize input buffer
    queue_init();

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


PUBLIC bool spi_irq_setup_init(void)
{
    // Disable SPI IRQ while configuring
    irq_set_enabled(SPI0_IRQ, false);
    irq_set_priority(SPI0_IRQ,0);

    // Mask ONLY the interrupts we want
    spi0_hw->imsc =
          RXIM_BIT    // RX FIFO interrupt (RNE)
        | RORIM_BIT;  // RX overrun error

    // Register SPI IRQ handler
    irq_set_exclusive_handler(SPI0_IRQ, irq_handler);

    // Enable SPI IRQ
    irq_set_enabled(SPI0_IRQ, true);

    return irq_is_enabled(SPI0_IRQ);
}


PUBLIC void irq_processing_main( uint8_t *in_buf ) {
    int count = 0;
    int rear = return_rear();
    uint8_t x;

    for( count; count <=rear  ; count++ )
    {
         if( !dequeue( &x ))
         {
            break; //if false, break out of the loop
         } 

        in_buf[count++] = (uint8_t)x;

    }

    set_queue_empty();
   

   // comment for no reason
}




PRIVATE void irq_handler(void) {
    // Keep reading SPI data while there’s something available
    while (spi_get_hw(spi0)->sr & ( SPI_SSPSR_RNE_BITS )) { // SPI Fifo not empty
       // uint16_t word = *spi_data_reg & 0xFFFF;
        uint16_t word = spi_get_hw(spi0)->dr & SPI_SSPDR_DATA_BITS;

       if (!queue_is_full()) {
            enqueue(word);
        }
     }
       spi_get_hw(spi0)->icr =  SPI_SSPICR_RTIC_BITS | SPI_SSPICR_RORIC_BITS;
       spi_reading = false;
       spi_rx_len = BUF_LEN;

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





