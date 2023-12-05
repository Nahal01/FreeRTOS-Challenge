#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

// global configurations
#define BACKGROUND_CORE 0
#define UART_BAUD_RATE         112500  //UART baud rate [Bps]
#define XILO_TASK_UPDATE_FREQ   20     //Xylophone Task update rate [Hz]
#define XILO_TASK_DELAY_TICKS  ((1000/XILO_TASK_UPDATE_FREQ) / portTICK_PERIOD_MS) // Xylophone task delay between runs [ticks]
#define SWITCH_TASK_DELAY_TICKS (25 / portTICK_PERIOD_MS)  // switch task delay between runs [ticks] (assumes up to 1000/25=40 keystroke per seconds)

// pins definition
#define SWITCH_PIN 1 // connect switch to GPIO 1

//forward declarations
void setup();
void vXylophoneTask( void * pvParameters );
void vSwitchTask( void * pvParameters );
uint8_t uGetLastXyloCounter();

//global variables
static uint8_t g_uXyloCounter = 0;
static int     g_uSwitchStatus = 0;
static TaskHandle_t g_xXylophoneHandle = NULL;

int main(void)
{
  uint8_t uXyloCounterPrev = 0xFF; // init with an invalid number to print first time
  setup();

  while(1) {
    //read g_uXyloCounter value and print if it has been changed
    const uint8_t uXyloCounterCurr = uGetLastXyloCounter();
    if(uXyloCounterCurr != uXyloCounterPrev) {
      uXyloCounterPrev = uXyloCounterCurr;
      Serial.print("uXyloCounter changed to ");
      Serial.println(uXyloCounterCurr);
    }

    if(g_uSwitchStatus == HIGH) {
      //pause vXylophoneTask
       vTaskSuspend( g_xXylophoneHandle );
    } else {
      //resume vXylophoneTask
      vTaskResume( g_xXylophoneHandle );
    }
  }

  return 0;
}

//Initialization
void setup()
{
  g_uXyloCounter  = 0;
  g_uSwitchStatus = 0;
  Serial.begin(UART_BAUD_RATE);

  // initialize the switch pin as an input
  pinMode(SWITCH_PIN, INPUT);
  
  xTaskCreatePinnedToCore(
                      vXylophoneTask,     /* Function to implement the task */
                      "XylophoneTask",    /* Name of the task */
                      128,               /* Stack size in words */
                      NULL,               /* Task input parameter */
                      0,                  /* Priority of the task */
                      &g_xXylophoneHandle,/* Task handle. */
                      BACKGROUND_CORE);   /* Core where the task should run */

  xTaskCreate(
              vSwitchTask,          /* Task function. */
              "SwitchTask",         /* String with name of task. */
              128,                  /* Stack size in bytes. */
              NULL,                 /* Parameter passed as input of the task */
              1,                    /* Priority of the task. */
              NULL);                /* Task handle. */
}

uint8_t uGetLastXyloCounter()
{
  return g_uXyloCounter;
}

void vXylophoneTask( void * pvParameters )
{
  TickType_t xLastWakeTime;

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();

  for( ;; )
  {
    // Wait for the next cycle.
    vTaskDelayUntil( &xLastWakeTime, XILO_TASK_DELAY_TICKS );

    //increment counter
    g_uXyloCounter = (g_uXyloCounter > 100) ? 0 : g_uXyloCounter + 1;
  }
}

void vSwitchTask( void * pvParameters )
{
  TickType_t xLastWakeTime;

  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();

  for( ;; )
  {
    //read switch pin value
    g_uSwitchStatus = digitalRead(SWITCH_PIN);

    // Wait for the next cycle.
    vTaskDelayUntil( &xLastWakeTime, SWITCH_TASK_DELAY_TICKS );
  }
}
