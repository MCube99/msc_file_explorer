#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <string.h>


struct queue_type {
    volatile unsigned int front;          // read index
    volatile unsigned int rear;           // write index
    uint16_t buffer[ BUF_LEN ];
}; 

static struct queue_type myQueue;
struct queue_type *queue_pointer = &myQueue;


// The above is for functionality where a static queue is used. This is only for this source file. 

PUBLIC void queue_init( ) {
    myQueue.front = 0;                // start read index at 0
    myQueue.rear = 0;                 // start write index at 0
    memset(myQueue.buffer, 0, sizeof(myQueue.buffer)); // initialize all buffer elements to 0
}




PUBLIC bool enqueue( uint16_t x ) {

    if((queue_pointer->rear + 1) % BUF_LEN == queue_pointer->front ) { //loops back to start if at end
        return false; // buffer full
    }

    else 
    {
        queue_pointer->rear = (queue_pointer->rear + 1 )%BUF_LEN;
        queue_pointer->buffer[queue_pointer->rear] = x;
        return true;
    }

}



PUBLIC bool dequeue( uint16_t *x ) {
    if ( queue_pointer->front == queue_pointer->rear ) {
        return false; // buffer empty
    }

    else { 
        queue_pointer->front = (queue_pointer->front+1)%BUF_LEN;
        *x = queue_pointer->buffer[myQueue.front];
        return true; 
    }
    
}


PUBLIC bool set_queue_empty() {
        queue_init();
 
}

PUBLIC bool is_queue_full() {
    return ((myQueue.rear + 1) % BUF_LEN == myQueue.front); //should wrap around after BUF_LEN to back to 0

}

PUBLIC bool is_queue_empty(void)
{
    return (myQueue.front == myQueue.rear);
}

PUBLIC uint16_t* return_buffer()
{
    return ( myQueue.buffer );
}


PUBLIC unsigned int return_front()
{
    return( myQueue.front);
}

PUBLIC unsigned int return_rear()
{
    return( myQueue.rear);
}

// End of queue.c file
