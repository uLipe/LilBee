/**
 * @brief event queue interface file
 */
#include "event_queue.h"

/**
 * private variables
 */
static int put_index = 0;
static int get_index = 0;
static int noof_elements = 0;
static system_event_t event_queue[EVENT_QUEUE_LEN] = {k_noevent};


/**
 * public functions
 */
int event_queue_init(void)
{
   /* not needed */
   return 0;
}
system_event_t event_queue_peek(void)
{
    system_event_t ret = (noof_elements == 0)? k_noevent : event_queue[get_index];
    return(ret);
}
system_event_t event_queue_get(void)
{
    system_event_t ret = (noof_elements == 0)? k_noevent : event_queue[get_index];
    if(ret != k_noevent) {
        get_index = (get_index + 1) % EVENT_QUEUE_LEN;
        noof_elements--;
    }
    return(ret);
}
int event_queue_put(system_event_t ev)
{
    int ret = 0;

    if(noof_elements < EVENT_QUEUE_LEN) {
        /* send event to the most back position of queue */
        event_queue[put_index] = ev;
        put_index = (put_index + 1) % EVENT_QUEUE_LEN;
        noof_elements++;
    } else {
        /* event queue full */
        ret = -1;
    }

    return(ret);
}
