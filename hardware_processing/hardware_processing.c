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
#include "hardware/structs/iobank0.h"


#define spi_default                         spi0
#define PICO_DEFAULT_SPI_SCK_PIN            2   // SPI clock, same as master
#define PICO_DEFAULT_SPI_RX_PIN             4
#define PICO_DEFAULT_SPI_TX_PIN             3
#define PICO_DEFAULT_SPI_CSN_PIN            5 
#define DEBUG_PIN                           6
// #define PICO_DEFAULT_SPI_SCK_PIN            2   // SPI clock, same as master
// #define PICO_DEFAULT_SPI_RX_PIN             0   // MOSI from master → receive on slave
// #define PICO_DEFAULT_SPI_TX_PIN             3   // MISO to master → send from slave
// #define PICO_DEFAULT_SPI_CSN_PIN            1   // Chip select
// #define SPI_IRQ_PIN                         6   // GPIO pin for SPI interrupt line from master


uint8_t test[256] = {0};

PRIVATE void spi0_irq_handler();
PRIVATE void csn_irq(uint gpio, uint32_t events);

PUBLIC void prepare_memory_for_spi_transfer( uint8_t *buffer, uint8_t *copy_buffer ) {

    // Placeholder for any memory preparation needed before SPI transfer
    memset(buffer, 0 , NUMBER_OF_BYTES); // Initialize input buffer
    memset(copy_buffer, 0, NUMBER_OF_BYTES);
    queue_init();

}

PUBLIC void set_spi_gpio_pins() {

    // 1.263Mhz or 2MHZ
    spi_init(spi0, 1*1000*1000);

    spi_set_format(
    spi0,
    16,                    // bits
    SPI_CPOL_0,
    SPI_CPHA_0,
    //SPI_MSB_FIRST
    SPI_MSB_FIRST
    );

    spi_set_slave(spi0, true);


    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
    gpio_set_function(DEBUG_PIN, GPIO_FUNC_SIO);

    
    gpio_set_dir(DEBUG_PIN, false);
    gpio_pull_up(DEBUG_PIN);

    // gpio_set_irq_enabled_with_callback(DEBUG_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE ,true, &spi0_irq_handler);

    // | GPIO_IRQ_EDGE_RISE | GPIO_IRQ_LEVEL_HIGH
   // irq_set_priority(SIO_IRQ_PROC0,1);

}


PUBLIC bool spi_irq_setup_init(void)
{
    irq_set_enabled(SPI0_IRQ, false);
     irq_set_priority(SPI0_IRQ, 0);      // Lower priority

    // Clear any pending SPI interrupts
    hw_set_bits(&spi_get_hw(spi0)->icr, SPI_SSPICR_RTIC_BITS | SPI_SSPICR_RORIC_BITS);

    // Clear interrupt mask
    hw_clear_bits(&spi_get_hw(spi0)->imsc, SPI_SSPIMSC_BITS);

    // Enable desired SPI interrupt sources
    hw_set_bits(&spi_get_hw(spi0)->imsc,
                SPI_SSPIMSC_RTIM_BITS | SPI_SSPIMSC_RXIM_BITS | SPI_SSPIMSC_RORIM_BITS);
      int array_check[32] = {0};
      uint32_t locks =sio_hw->spinlock_st;
      for (int i = 0; i < 32; i++) {
        if (locks & (1u << i)) {
            array_check[i] = 1;
        }
      }

//         if(array_check[i] == 1) {
//             spin_lock_t *lock = spin_lock_instance(i); // Example: Lock 0
//             int32_t saved_irq = spin_lock_blocking(lock);
//             spin_unlock(lock, saved_irq);
//         }
//     }

    // Assign SPI0 IRQ handler **before enabling the IRQ**
    irq_set_exclusive_handler(SPI0_IRQ, spi0_irq_handler);
    irq_set_enabled(SPI0_IRQ, true);

    return irq_is_enabled(SPI0_IRQ);
}




// PUBLIC void irq_processing_main( uint8_t *in_buf ) {
//     static int count = 0;
//     uint16_t x;

//     for( ; count <= BUF_LEN ; count++ )
//     {
//          if( !dequeue( &x ))
//          {
//             break;
//          } 
//         in_buf[count++] = x;

//     }

//     if(count > BUF_LEN)
//      {
//         count = 0;
//      }

//     set_queue_empty();
   

//    // comment for no reason
// }


PRIVATE void spi0_irq_handler() {

 
        uint32_t status = spi_get_hw(spi0)->mis;
        while (spi_is_readable(spi0)) {
            uint32_t word = spi_get_hw(spi0)->dr & SPI_SSPDR_DATA_BITS;
            if(status & SPI_SSPMIS_RXMIS_BITS) 
            {
                enqueue(word);
                hw_clear_bits(&spi_get_hw(spi0)->icr, SPI_SSPICR_RTIC_BITS | SPI_SSPICR_RORIC_BITS);
            }
            if(status & SPI_SSPMIS_RTMIS_BITS)
            {
                enqueue(word);
                hw_clear_bits(&spi_get_hw(spi0)->icr, SPI_SSPICR_RTIC_BITS | SPI_SSPICR_RORIC_BITS);
            }
            
            if(status & SPI_SSPMIS_RORMIS_BITS)
            {
                enqueue(word);
                hw_clear_bits(&spi_get_hw(spi0)->icr, SPI_SSPICR_RTIC_BITS | SPI_SSPICR_RORIC_BITS);
            }
        }
}


       // spi_read_blocking(spi0, 0x1,test,BUF_LEN);
         
        // Here, word will contain 8-bit data in LSB
       



// GPIO CSN IRQ handler in SRAM
PRIVATE void csn_irq(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL ) {
        csn_low = true;
    }
    if (events & GPIO_IRQ_EDGE_RISE | GPIO_IRQ_LEVEL_HIGH  ) {
        csn_low = false;
     //   hw_clear_bits(&spi_get_hw(spi0)->icr, SPI_SSPICR_RTIC_BITS | SPI_SSPICR_RORIC_BITS);

    }
}

    
    
    
 PUBLIC void copy_queue_buffer( void ) {

    uint16_t x = 0;
    static uint8_t count = 0;
   // uint32_t status = save_and_disable_interrupts();
    while(dequeue(&x)) //while queueu is not empty
    {
       // dest[count++] = x;
       append_char(x);
    
    }

     if( count == BUF_LEN )
     {
         count = 0;
         set_queue_empty();
     }

//     restore_interrupts(status);
}




PUBLIC void gpio_set_irq_active(uint gpio, uint32_t events, bool enabled) {
    io_bank0_irq_ctrl_hw_t *irq_ctrl_base = get_core_num() ?  
    &io_bank0_hw->proc1_irq_ctrl : &io_bank0_hw->proc0_irq_ctrl;
    io_rw_32 *en_reg = &irq_ctrl_base->inte[gpio / 8];
    events <<= 4 * (gpio % 8);
    if (enabled)
    {
        hw_set_bits(en_reg, events);
    }
    else
    {
        hw_clear_bits(en_reg, events);
    }
}

PUBLIC bool spi_is_processing() {

    return(spi_is_busy(spi0));

    // if 1 still busy , if 0 its done transmitting

}

