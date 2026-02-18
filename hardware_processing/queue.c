#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <string.h>
#include "hardware/sync.h"


struct queue_type {
    volatile unsigned int front;          // read index
    volatile unsigned int rear;           // write index
    DWORD buffer[ BUF_LEN ];
}; 

static struct queue_type myQueue;



// The above is for functionality where a static queue is used. This is only for this source file. 

PUBLIC void queue_init( ) {
    myQueue.front = 0;                // start read index at 0
    myQueue.rear = 0;                 // start write index at 0

    memset(myQueue.buffer, 0, BUF_LEN); // initialize all buffer elements to 0
}




PUBLIC bool enqueue( uint32_t x ) {

    if((myQueue.rear + 1) % BUF_LEN == myQueue.front ) { //loops back to start if at end
        return false; // buffer full
    }

    else 
    {
        myQueue.buffer[myQueue.rear] = x;
        myQueue.rear = (myQueue.rear + 1) % BUF_LEN; // moving to next index
        return true;
    }

}



PUBLIC bool dequeue( uint32_t *x ) {
    if ( myQueue.front == myQueue.rear ) {
        return false; // buffer empty
    }

    else {
        *x = myQueue.buffer[myQueue.front];
         myQueue.front = (myQueue.front + 1) % BUF_LEN; // The modulus is a condition to get it to move forward until the end and then loop back to start
        
        return true;
       
        
    }
    
}


PUBLIC bool set_queue_empty() {

    if(queue_is_empty)
    {
        queue_init();
    }
    
}

PUBLIC bool queue_is_full() {
    bool is_queue_full;
    uint32_t status = save_and_disable_interrupts();
    is_queue_full = (myQueue.rear + 1) % BUF_LEN == myQueue.front; //should wrap around after BUF_LEN to back to 0
    restore_interrupts(status );
    return(is_queue_full);
}

PUBLIC bool queue_is_empty(void)
{
    return (myQueue.front == myQueue.rear);
}




PUBLIC int GetBytesAvailable()
{
    int result;
    uint32_t status = save_and_disable_interrupts();
    result = myQueue.rear - myQueue.front;
    restore_interrupts(status);
    if(result < 0) {
        result+= BUF_LEN;
    }
    return(result);
}

PUBLIC int GetNextByte()
{
    int result = -1;
    uint32_t status = save_and_disable_interrupts();
    if(myQueue.front != myQueue.rear)
    {
        result = myQueue.buffer[myQueue.front++];
        myQueue.front%=BUF_LEN;
    }
    restore_interrupts(status);
    return result;
}

// End of queue.c file
