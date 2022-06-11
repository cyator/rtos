#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint8_t message_queue_length = 5;
static QueueHandle_t msg_queue;

void ReadMessage(void *pvParameters)
{
    while (1)
    {

        if (Serial.available() > 0)
        {
            String s = Serial.readStringUntil('\n');

            if (xQueueSend(msg_queue, (void *)&s, 10) != pdTRUE)
            {
                Serial.println("Queue full");
            }
        }
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void PrintMessage(void *pvParameters)
{

    String s;

    while (1)
    {

        if (xQueueReceive(msg_queue, (void *)&s, 0) == pdTRUE)
        {
            Serial.println(s);
        }

        // delay before attempting message retrieval from queue
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void setup()
{

    Serial.begin(115200);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    msg_queue = xQueueCreate(message_queue_length, 255 * sizeof(char));

    Serial.println();
    Serial.println("___Queue demo___");

    Serial.println("Enter string: ");

    xTaskCreatePinnedToCore(
        ReadMessage,   // function to be called
        "ReadMessage", // name for task
        1024,          // This stack size in bytes
        NULL,          // pass parameter to function
        1,             // Priority 0 lowest 24 highest.
        NULL,          // task handle
        app_cpu        // run on core 1
    );
    xTaskCreatePinnedToCore(
        PrintMessage,   // function to be called
        "PrintMessage", // name for task
        1024,           // This stack size in bytes
        NULL,           // pass parameter to function
        1,              // Priority 0 lowest 24 highest.
        NULL,           // task handle
        app_cpu         // run on core 1
    );

    //   // delete setup and loop task
    vTaskDelete(NULL);
}

void loop()
{
}
