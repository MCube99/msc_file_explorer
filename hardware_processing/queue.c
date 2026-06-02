#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "queue.h"

#include "hardware/dma.h"

#include "hardware_processing.h"
#include "hardware/uart.h"
#include "hardware/sync.h"

// -----------------------------------------------------------------------------
// EXTERN VARIABLE STORAGE
// -----------------------------------------------------------------------------
 volatile packet_type_t current_packet = PACKET_START;

// -----------------------------------------------------------------------------
// QUEUE STORAGE
// -----------------------------------------------------------------------------

struct queue_type {
    BYTE buffer[BUF_LEN];
};

static struct queue_type myQueue;

// -----------------------------------------------------------------------------
// QUEUE INITIALIZATION
// -----------------------------------------------------------------------------

PUBLIC void queue_init(void) {
    memset(myQueue.buffer, 0, BUF_LEN);
}

// -----------------------------------------------------------------------------
// BUFFER ACCESSORS
// -----------------------------------------------------------------------------

PUBLIC uint8_t *give_array_address(void) {
    return &myQueue.buffer[0];
}

PUBLIC uint8_t *give_array_address_for_file_writing(void) {
    return &myQueue.buffer[1];
}

PUBLIC int get_queue_size(void) {
    return myQueue.buffer[0];
}

// -----------------------------------------------------------------------------
// PACKET CLASSIFICATION
// -----------------------------------------------------------------------------

PUBLIC void classify_packet(void) {

    uint32_t size = return_size();
    if(size == 127) {
        size = 254; // this is because the PIO state machine returns 127 when the actual size is 0, so we need to correct for that. This is a quirk of the PIO state machine and how it handles the size byte, which is why we need to check for this specific value and set it to 0 if we see it.
    }

    switch( size ) {
        case 0:
        case GARY_CODE:
            current_packet = PACKET_KEYBOARD;
            gpio_put(PICO_DEFAULT_KEYBOARD_PIN, 1); // Set the pin high to signal that we are processing a keyboard packet
            keyboard_check = false;
            break;

        default: // this is for the USB packet, which is the only other option
            gpio_put(PICO_DEFAULT_KEYBOARD_PIN, 0); // Set the pin high to signal that we are processing a usb packet
            current_packet = PACKET_USB;
            usb_check = true;
            pio_sm_put(return_spi_pio(), return_spi_sm(), return_size()); //puts the first byte of the packet into the PIO state machine for processing
            dma_start_channel_mask(1u << return_channel()); 
    }


    size_byte_set = false; // reset the flag for the next packet
}


PUBLIC void check_usb_transfer() {

    uintptr_t base = (uintptr_t)&myQueue.buffer[0];

    uintptr_t write = dma_hw->ch[return_channel()].write_addr;

    uint32_t difference = write - base;

    if(difference ==  return_size())
    {
        usb_transfer_done = true;
        current_packet = PACKET_NONE;
    }
}
