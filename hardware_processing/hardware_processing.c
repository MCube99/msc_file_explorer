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
#include "file_processing.h"
#include "queue.h"
#include "hardware/sync.h"
#include "pico/time.h"
#include "hardware/structs/iobank0.h"

#define spi_default                         spi0
#define PICO_DEFAULT_SPI_SCK_PIN            2   // SPI clock, same as master
#define PICO_DEFAULT_SPI_RX_PIN             4
#define PICO_DEFAULT_SPI_TX_PIN             3
#define PICO_DEFAULT_SPI_CSN_PIN            5    
#define DEBUG_PIN                            6
#define EVT_SPI_0_RX                        1

//#define PICO_DEFAULT_SPI_SCK_PIN            2   // SPI clock, same as master
//#define PICO_DEFAULT_SPI_RX_PIN             0   // MOSI from master → receive on slave
//#define PICO_DEFAULT_SPI_TX_PIN             3   // MISO to master → send from slave
//#define PICO_DEFAULT_SPI_CSN_PIN            1   // Chip select
//   // GPIO pin for SPI interrupt line from master



PRIVATE void convert_hex_to_string(const uint8_t *byte_array, int length, char *conversion);
PRIVATE void myIRQHandler(uint gpio, uint32_t events);






PUBLIC void prepare_memory_for_spi_transfer(uint8_t *in_buf) {

    // Placeholder for any memory preparation needed before SPI transfer

    
    memset(in_buf, 0 , NUMBER_OF_BYTES); // Initialize input buffer
    queue_init();

}

 PUBLIC void copy_queue_buffer(void)
{
    int x = 0;
    static unsigned int count = 0;

    // gpio_set_irq_active(DEBUG_PIN,
    //                     GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
    //                     false);

    x = GetNextByte();
     if (x == 256)
     {
        set_queue_empty();
      }
    }

//     gpio_set_irq_active(DEBUG_PIN,
//                         GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
//                         true);
 



PRIVATE void gpio_clear_events(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio,events);
}


PRIVATE void myIRQHandler(uint gpio, uint32_t events) {
    uint16_t test[256] = {0};
    if( events & GPIO_IRQ_EDGE_FALL  )
    {
        while(spi_is_busy(spi0))
        {
             csn_high = true;
        }
       
    
        spi_reading = false;    //reading is done 
    }

    if( events & GPIO_IRQ_EDGE_RISE)
    {
        csn_high = false;
        spi_reading = true; 
    }

      gpio_clear_events(gpio,events);
}

PUBLIC void set_gpio_pins() {
    

    gpio_set_function(DEBUG_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(DEBUG_PIN, GPIO_IN);
    gpio_set_input_enabled(DEBUG_PIN, true); // interrupt line needs to detect input
    gpio_pull_up(DEBUG_PIN);
   
    gpio_set_irq_enabled_with_callback(DEBUG_PIN, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true, &myIRQHandler);


}

PUBLIC void set_spi_gpio_pins() {

    spi_init(spi0,  0); 
    spi_set_slave(spi0, true);

    spi_set_format(
    spi0,
    8,                    // bits
    SPI_CPOL_0,
    SPI_CPHA_0,
    SPI_LSB_FIRST
    );

    spi_set_slave(spi0, true);


    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);
    

}




PUBLIC void gpio_set_irq_active(uint gpio, uint32_t events, bool enabled) {
    io_bank0_irq_ctrl_hw_t *irq_ctrl_base = get_core_num() ? &io_bank0_hw->proc1_irq_ctrl : &io_bank0_hw->proc0_irq_ctrl;
    io_rw_32 *en_reg = &irq_ctrl_base-> inte[gpio/8];
    events<<= 4 * (gpio%8);
    if(enabled)
    {
        hw_set_bits(en_reg,events);
    }
    else
    {
        hw_clear_bits(en_reg, events);
    }
}



PUBLIC bool spi_irq_setup_init(void)
{
    
    spi_get_hw(spi0)->imsc = 0;


    spi_get_hw(spi0)->icr = SPI_SSPICR_RTIC_BITS | SPI_SSPICR_RORIC_BITS;


    irq_set_exclusive_handler(SPI0_IRQ, spi0_irq_handler);
    irq_set_priority(SPI0_IRQ,0);

    irq_set_enabled(SPI0_IRQ, true);


    spi_get_hw(spi0)->imsc =
        SPI_SSPIMSC_RTIM_BITS |
        SPI_SSPIMSC_RXIM_BITS |
        SPI_SSPIMSC_RORIM_BITS;

    return true;
}






PUBLIC void spi0_irq_handler(void)
{
    uint32_t status;
     uint32_t word;

   

    while ((status = spi_get_hw(spi0)->mis) &
           (SPI_SSPMIS_RXMIS_BITS |
            SPI_SSPMIS_RTMIS_BITS |
            SPI_SSPMIS_RORMIS_BITS))
    {
        // Drain RX FIFO
        if (spi_is_readable(spi0))
        {
            word = spi_get_hw(spi0)->dr;
         //   uint16_t word = spi_get_hw(spi0)->dr & SPI_SSPDR_DATA_BITS;
             enqueue(word); 
            csn_high = true;
        }

        // Clear timeout + overrun
        spi_get_hw(spi0)->icr =
            SPI_SSPICR_RTIC_BITS |
            SPI_SSPICR_RORIC_BITS;
    }

    csn_high = false;
}




PUBLIC bool spi_is_processing() {

    return(spi_is_busy(spi0));

    // if 1 still busy , if 0 its done transmitting

}

