/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/* Example to show how to navigate mass storage device with built-in command line.
 * Type help for list of supported commands and syntax (mostly linux commands)

 > help
 * help
        Print list of commands
 * cat
        Usage: cat [FILE]...
        Concatenate FILE(s) to standard output..
 * cd
        Usage: cd [DIR]...
        Change the current directory to DIR.
 * cp
        Usage: cp SOURCE DEST
        Copy SOURCE to DEST.
 * ls
        Usage: ls [DIR]...
        List information about the FILEs (the current directory by default).
 * pwd
        Usage: pwd
        Print the name of the current working directory.
 * mkdir
        Usage: mkdir DIR...
        Create the DIRECTORY(ies), if they do not already exist..
 * mv
        Usage: mv SOURCE DEST...
        Rename SOURCE to DEST.
 * rm
        Usage: rm [FILE]...
        Remove (unlink) the FILE(s).
 */

#include <stdio.h>
#include <stdbool.h>
#include "bsp/board_api.h"
#include "tusb.h"

#include "msc_app.h"
#include "file_processing.h"
#include "hardware_processing.h"
#include "queue.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/watchdog.h" 
#include "hardware/structs/sio.h"
#define FALSE 0
#define TRUE  1



 // 


//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
void led_blinking_task(void);

/// Defining Flags ////

volatile bool csn_low = false;



/*------------- MAIN -------------*/
int main(void) {

   
  uint8_t in_buf[ BUF_LEN ];
  uint8_t copy_buffer[ BUF_LEN ];

  




  stdio_init_all();
  timer_hw->dbgpause = 0; 
    
  
  board_init();
  set_spi_gpio_pins();
  spi_irq_setup_init();
  
  prepare_memory_for_spi_transfer( in_buf, copy_buffer );

  

    // this is so that the interrupts don't fire up over here when spi stuff is being set up
//   prepare_memory_for_spi_transfer();
//   ;

//   bool spi_check = spi_irq_setup_init();

//   if(spi_check == TRUE)
//   {
//       spi_transfer = TRUE;
//   }
  
  

  // init host stack on configured roothub port
  tusb_rhport_init_t host_init = {
    .role = TUSB_ROLE_HOST,
    .speed = TUSB_SPEED_AUTO
  };
  tusb_init(BOARD_TUH_RHPORT, &host_init);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  msc_app_init();


  while (1) {
    // tinyusb host task
    //if spi is true, then its still reading, hence don't go to the bottom
     
       tuh_task();
       msc_app_task();
       led_blinking_task();
     
// csn_low &&
      if( !is_queue_empty())
      {
        copy_queue_buffer();
        if(!spi_is_processing())
        {
           file_processing_main();
        }
       
      }
     
     
     

    
    
     //gpio_set_irq_active(DEBUG_PIN, GPIO_IRQ_EDGE_RISE, false);
     
     //gpio_set_irq_active(DEBUG_PIN, GPIO_IRQ_EDGE_FALL, true);
    //  
     
  }

  return 0;
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

void tuh_mount_cb(uint8_t dev_addr) {
  (void) dev_addr;
}

void tuh_umount_cb(uint8_t dev_addr) {
  (void) dev_addr;
}

//--------------------------------------------------------------------+
// Blinking Task
//--------------------------------------------------------------------+
void led_blinking_task(void) {
  const uint32_t interval_ms = 1000;
  static uint32_t start_ms = 0;

  static bool led_state = false;

  // Blink every interval ms
  if (board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}

// Simple delay implementation using Pico SDK (or your platform)
void tusb_time_delay_ms_api(uint32_t ms) {
    sleep_ms(ms);   // Pico SDK function
}

// HID report callback
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    // Handle received HID report here
}

//--------------------------------------------------------------------+
// Array processign tasks
//--------------------------------------------------------------------+


