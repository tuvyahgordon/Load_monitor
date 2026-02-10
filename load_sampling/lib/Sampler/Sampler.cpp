#include <Arduino.h>
#include "Sampler.h"
#include "Common.h"

/* ================== Configuration ================== */

static constexpr uint32_t SAMPLE_RATE_HZ = 4000;
static constexpr uint32_t SAMPLE_PERIOD_US = 1000000 / SAMPLE_RATE_HZ; // period in seconds
static constexpr int ADC_PIN = 34;   // to change if needed
static constexpr uint32_t WINDOW_SAMPLES = 4000; // 1 second

/* ================== Internal initial state ================== */

static uint32_t n = 0;
static double samp_sum = 0.0;
static double samp_sum_sqrd = 0.0;

/* ================== Sampler task ================== */

void samplerTask(void* arg) // freeRTOS task, arg will be NULL
{
    // Configure ADC
    analogReadResolution(12);        // ESP32 default
    analogSetAttenuation(ADC_11db);  // to prevent clipping 

    QueueHandle_t q = (QueueHandle_t)arg;
    uint32_t next_sample_time = micros(); // initiate to current time

    for (;;)
    {
        // Busy-wait until next sample time
        uint32_t now = micros(); 
        if ((int32_t)(now - next_sample_time) >= 0)
        {
            next_sample_time += SAMPLE_PERIOD_US;

            // ---- Sample ----
            int raw = analogRead(ADC_PIN);

            // Convert to double for accumulation
            double v = (double)raw;

            // ---- Accumulate ----
            n++;
            samp_sum += v;
            samp_sum_sqrd += v * v;

             if (n >= WINDOW_SAMPLES) // snapshot and queue
             {   
                StatsSnapshot snap{ n, samp_sum, samp_sum_sqrd };
                xQueueSend(q, &snap, 0);   // non-blocking

                n = 0;
                samp_sum = 0.0;
                samp_sum_sqrd = 0.0;
             }

        // Yield so other tasks can run
        taskYIELD();
        }
    }
}
