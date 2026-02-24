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
#include "clocked_input.pio.h"

#define spi_default                         spi0
#define PICO_DEFAULT_SPI_SCK_PIN            2   // SPI clock, same as master
#define PICO_DEFAULT_SPI_RX_PIN             4
#define PICO_DEFAULT_SPI_TX_PIN             3
#define PICO_DEFAULT_SPI_CSN_PIN            5    
#define DEBUG_PIN                           6

typedef struct pio_spi_inst {
    PIO pio;
    uint sm;
    uint cs_pin;
    uint offset;
} pio_spi_inst_t;

static pio_spi_inst_t pio_spi = {
    .pio = pio0,
    .sm = 0,
    .cs_pin = PICO_DEFAULT_SPI_CSN_PIN
};



//#define PICO_DEFAULT_SPI_SCK_PIN            2   // SPI clock, same as master
//#define PICO_DEFAULT_SPI_RX_PIN             0   // MOSI from master → receive on slave
//#define PICO_DEFAULT_SPI_TX_PIN             3   // MISO to master → send from slave
//#define PICO_DEFAULT_SPI_CSN_PIN            1   // Chip select
//   // GPIO pin for SPI interrupt line from master




PRIVATE void myIRQHandler();






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


PUBLIC bool spi_irq_setup_init(void)
{
    
    

    spi_get_hw(spi0)->icr = SPI_SSPICR_RTIC_BITS | SPI_SSPICR_RORIC_BITS;
    spi_get_hw(spi0)->imsc =
        SPI_SSPIMSC_RXIM_BITS; 

    return true;
}



PUBLIC void setupPIO(void)
{
     spi_get_hw(spi0)->icr = SPI_SSPICR_RTIC_BITS | SPI_SSPICR_RORIC_BITS;
    spi_get_hw(spi0)->imsc =
        SPI_SSPIMSC_RXIM_BITS;

    uint offset = pio_add_program(pio_spi.pio, &clocked_input_program);
    uint sm = pio_claim_unused_sm(pio_spi.pio, true);
    clocked_input_program_init(pio_spi.pio, sm, offset,PICO_DEFAULT_SPI_RX_PIN);
   // Setup PIO interrupt handling for clocked input
    pio_set_irq0_source_mask_enabled(pio_spi.pio, pis_interrupt0|pis_sm0_rx_fifo_not_empty|pis_sm1_rx_fifo_not_empty|pis_sm2_rx_fifo_not_empty|pis_sm3_rx_fifo_not_empty, true);
    irq_set_exclusive_handler(PIO0_IRQ_0, myIRQHandler);
    irq_set_priority(PIO0_IRQ_0, 1);
    irq_set_enabled(PIO0_IRQ_0, true);
}

PRIVATE void myIRQHandler()
{
     uint32_t status;
     uint32_t word;
 
    // while ((spi_is_readable(spi0)))
    // {
    //         word = pio_sm_get(pio_spi.pio, pio_spi.sm); // Read data from PIO state machine FIFO
    //         enqueue(word); 
    //         spi_reading = true;
    // }

        
            word = pio_sm_get(pio_spi.pio, pio_spi.sm); // Read data from PIO state machine FIFO
            enqueue(word); 

            if(!queue_is_full())
            {
                spi_reading = false;
            }



}