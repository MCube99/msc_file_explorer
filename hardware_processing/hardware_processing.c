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
#include "hardware/clocks.h"
#include "file_processing.h"
#include "queue.h"
#include "hardware/sync.h"
#include "pico/time.h"
#include "hardware/structs/iobank0.h"
#include "clocked_input.pio.h"

#define spi_default                         spi0
#define PICO_DEFAULT_SPI_SCK_PIN            3   // SPI clock, same as master
#define PICO_DEFAULT_SPI_RX_PIN             2
#define PICO_DEFAULT_SPI_TX_PIN             5
#define PICO_DEFAULT_SPI_CSN_PIN            4    
#define DEBUG_PIN                           6


//#define PICO_DEFAULT_SPI_SCK_PIN            2   // SPI clock, same as master
//#define PICO_DEFAULT_SPI_RX_PIN             0   // MOSI from master → receive on slave
//#define PICO_DEFAULT_SPI_TX_PIN             3   // MISO to master → send from slave
//#define PICO_DEFAULT_SPI_CSN_PIN            1   // Chip select
//   // GPIO pin for SPI interrupt line from master

typedef struct {
    PIO pio;
    uint sm;
} pio_spi_t;

static pio_spi_t pio_spi; 


//PRIVATE void myIRQHandler(uint gpio, uint32_t events);
PRIVATE void myIRQHandler();


PRIVATE void gpio_clear_events(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio,events);
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


 

 

  PRIVATE void myIRQHandler() 
 {
  
            uint8_t word = (uint8_t)pio_sm_get(pio_spi.pio, pio_spi.sm); // Read data from PIO state machine FIFO
            enqueue(word); // Add data to queue
             csn_high = false;    
  } 


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
 
PUBLIC void set_gpio_pins() 
{


  spi_init(spi0, 1000 * 1000);
  uint actual_freq_hz = spi_set_baudrate(spi0, clock_get_hz(clk_sys) / 6);
  gpio_set_irq_enabled_with_callback(PICO_DEFAULT_SPI_CSN_PIN, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true, &myIRQHandler);

}









PUBLIC void setupPIO(void)
{
    pio_spi.pio = pio0;

    uint offset = pio_add_program(pio_spi.pio, &clocked_input_program);
    pio_spi.sm = pio_claim_unused_sm(pio_spi.pio, true);
    clocked_input_program_init(pio_spi.pio, pio_spi.sm, offset,PICO_DEFAULT_SPI_RX_PIN);

   //Setup PIO interrupt handling for clocked input
    pio_set_irq0_source_mask_enabled(pio_spi.pio, pis_sm0_rx_fifo_not_empty|pis_sm1_rx_fifo_not_empty|pis_sm2_rx_fifo_not_empty|pis_sm3_rx_fifo_not_empty, true);
    irq_set_exclusive_handler(PIO0_IRQ_0, myIRQHandler);
    irq_set_priority(PIO0_IRQ_0, 0);
    irq_set_enabled(PIO0_IRQ_0, true);  

}
