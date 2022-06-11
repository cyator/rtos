#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint8_t buf_len = 255;
static char *msg_ptr = NULL;
static volatile uint8_t msg_flag = 0;

void ReadSerial(void *pvParameters)
{
  char c;
  char buf[buf_len];
  uint8_t index = 0;

  // Clear whole buffer
  memset(buf, 0, buf_len);

  while (1)
  {

    if (Serial.available() > 0)
    {

      c = Serial.read();

      if (index < buf_len - 1)
      {
        buf[index] = c;
        index++;
      }

      if (c == '\n')
      {
        // it with '\0' to make it null-terminated
        buf[index - 1] = '\0';

        // Try to allocate memory and copy over message. If message buffer is
        // still in use, ignore the entire message.
        if (msg_flag == 0)
        {
          msg_ptr = (char *)pvPortMalloc(index * sizeof(char));

          // If malloc returns 0 (out of memory), throw an error and reset
          configASSERT(msg_ptr);

          // Copy message
          memcpy(msg_ptr, buf, index);

          // Notify other task that message is ready
          msg_flag = 1;
        }
      }
    }
  }
}

void PrintSerial(void *pvParameters)
{
  while (1)
  {

    if (msg_flag == 1)
    {
      Serial.println(msg_ptr);

      Serial.print(" Before Free heap (bytes): ");
      Serial.println(xPortGetFreeHeapSize());

      // Free buffer, set pointer to null, and clear flag
      vPortFree(msg_ptr);

      msg_ptr = NULL;
      msg_flag = 0;

      Serial.print("After Free heap (bytes): ");
      Serial.println(xPortGetFreeHeapSize());
      Serial.println(msg_ptr);
    }
  }
}

void setup()
{

  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();
  Serial.println("__Serial echo__");

  // prompt for input
  Serial.println("Enter a string");

  xTaskCreatePinnedToCore(
      ReadSerial,   // function to be called
      "ReadSerial", // name for task
      1024,         // This stack size in bytes
      NULL,         // pass parameter to function
      1,            // Priority 0 lowest 24 highest.
      NULL,         // task handle
      app_cpu       // run on core 1
  );
  xTaskCreatePinnedToCore(
      PrintSerial,   // function to be called
      "PrintSerial", // name for task
      1024,          // This stack size in bytes
      NULL,          // pass parameter to function
      1,             // Priority 0 lowest 24 highest.
      NULL,          // task handle
      app_cpu        // run on core 1
  );

  // delete setup and loop task
  vTaskDelete(NULL);
}

void loop()
{
}
