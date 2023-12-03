
# Recommendations for better extension of the code

1. One possible issue with the current logic is that `vSwitchTask` has higher prioiry (in order not to miss any user request).
   So, it is possible for the main task to miss an event (e.g. a short pulse in `g_uSwitchStatus`). A better approach is to put
   suspension/resumption of `vXylophoneTask` inside switch task
 
2. in case there are multiple switches and sensors to be read, we can group them based on their relative resopnse time AND/OR 
   application requirements. For inputs which must be fetched more requently, we can create a separate task with higher priority.
   Also, we can use queues to store multiple reads before being consumed by others (probably slower tasks). In addition, it may be
   reasonable to use global variables that latches on to input changes so that consumer task doesn't miss it (For example, if there
   is a fault input pin, we can latch its value when it occurs on the producer task, and clear the latch when used in the consumer task).

   ```
    uint8_t g_uFault = 0;
    void vConsumerTask(...){
      ...
      if (g_uFault) {
        g_uFault = 0; //clear latch
        //do fault procedure
      }
    }
    
    void vProducerTask(...) {
      g_uFault |= digitalRead(...); // latch on rising edge
    }
   ```

3. Sensors/Switches with lower access frequency can also be placed in the main task

4. In case there is an strict requirement for jitter in time interval between `g_uXyloCounter`, we can set higher priority for `vXylophoneTask` or
   setup a timer and use timer intrrupt service routine to update `g_uXyloCounter`.

5. it is possible to use an interrupt service routine for sensors/switches and queues to transfer data to consumer tasks

