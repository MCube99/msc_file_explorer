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
#include "hardware/dma.h"

#define spi_default                         spi0
#define PICO_DEFAULT_SPI_SCK_PIN            3   // SPI clock, same as master
#define PICO_DEFAULT_SPI_RX_PIN             2
#define PICO_DEFAULT_SPI_TX_PIN             5
#define PICO_DEFAULT_SPI_CSN_PIN            4    
#define DEBUG_PIN                           6


#define PIO_SERIAL_CLKDIV                   10.f

//#define PICO_DEFAULT_SPI_SCK_PIN            2   // SPI clock, same as master
//#define PICO_DEFAULT_SPI_RX_PIN             0   // MOSI from master → receive on slave
//#define PICO_DEFAULT_SPI_TX_PIN             3   // MISO to master → send from slave
//#define PICO_DEFAULT_SPI_CSN_PIN            1   // Chip select
//   // GPIO pin for SPI interrupt line from master

typedef struct {
    PIO pio;
    uint sm;
    uint8_t data_bytes[ 256 ]; // Buffer to hold received data, size can be adjusted as needed
    int dma_chan; // DMA channel for PIO transfers
} pio_spi_t;

static pio_spi_t pio_spi; 




PRIVATE void dma_handler();


PRIVATE void gpio_clear_events(uint gpio, uint32_t events) {
    gpio_acknowledge_irq(gpio,events);
}


PRIVATE void dma_handler() {
    // Clear the DMA interrupt

    dma_hw->ints0 = 1u << pio_spi.dma_chan; // Clear the interrupt for channel 0
    
    dma_channel_set_write_addr(pio_spi.dma_chan, pio_spi.data_bytes,true); // Reset the write address for the next transfer

    csn_high = false; // Set the flag to indicate that CSN is low, meaning reading is happening


}


PUBLIC void prepare_memory_for_spi_transfer() {

    // Placeholder for any memory preparation needed before SPI transfer
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
 



PUBLIC void dma_setup(void)
{

     pio_spi.pio = pio0;
    uint offset = pio_add_program(pio_spi.pio, &clocked_input_program);
    pio_spi.sm = pio_claim_unused_sm(pio_spi.pio, true);
    clocked_input_program_init(pio_spi.pio, pio_spi.sm, offset,PICO_DEFAULT_SPI_RX_PIN,PIO_SERIAL_CLKDIV );

    pio_spi.dma_chan = dma_claim_unused_channel(true);
    dma_channel_config pio_dma_chan_config = dma_channel_get_default_config(pio_spi.dma_chan);
    //Tranfers 32-bits at a time
    channel_config_set_transfer_data_size(&pio_dma_chan_config, DMA_SIZE_32); //sets the size of each DMA transfer to 32 bits
    channel_config_set_read_increment(&pio_dma_chan_config, false); //Disabled when reading from peripheral, as the source address is fixed
    channel_config_set_write_increment(&pio_dma_chan_config, false); 
    channel_config_set_dreq(&pio_dma_chan_config, DREQ_PIO0_RX0); //Configures the DMA channel to be triggered by the PIO's RX FIFO for the specific state machine. This means that a DMA transfer will occur whenever there is data in the RX FIFO of the PIO state machine, allowing for efficient data handling without CPU intervention.
    dma_channel_configure(
        pio_spi.dma_chan, 
        &pio_dma_chan_config,
         NULL, // Destination address in memory where received data will be stored
         &pio_spi.pio->rxf[pio_spi.sm],
         256, 
         true); //Configure the DMA channel to read from the PIO RX FIFO and write to a destination buffer in memory. The destination address is set to NULL for now, as it will be updated dynamically during transfers. The transfer is started immediately by setting the last argument to true.


    dma_channel_set_irq0_enabled(pio_spi.dma_chan, true); //Enable the DMA channel's interrupt, allowing the CPU to be notified when a DMA transfer is complete. This is essential for handling the received data and performing any necessary processing after the transfer.

    //Configure the processor to run dma_handler() when DMA IRQ 0 is asserted
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true); //Enable DMA IRQ 0,

    //Manually call the handler once, to trigger the first DMA transfer and start the data flow from the PIO to memory. This is necessary because the DMA transfer is triggered by the PIO's RX FIFO, and we need to kickstart the process by initiating the first transfer.
    
}
