#include <Arduino.h>
#include "Sampler.h"
#include "Common.h"
#include "Config.h"

/* ================== Configuration ================== */

static constexpr uint32_t SAMPLE_PERIOD_US =
    1000000u / SAMPLE_RATE_HZ;

/* ================== Internal initial state ================== */


// Build-time CT pin list (CT2 only used if CT_COUNT > 1)
static constexpr int CT_PINS[MAX_CT] = {
    CT1_PIN,
#if CT_COUNT > 1
    CT2_PIN
#else
    -1
#endif
};

static uint32_t n = 0;
static double ct_sum[MAX_CT] = {0.0, 0.0}; // if i=only one CT, only first value will be used
static double ct_sum_sq[MAX_CT] = {0.0, 0.0};

#if HAS_VOLTAGE
static double v_sum = 0.0;
static double v_sum_sq = 0.0;
static double ct_sum_p[MAX_CT] = {0.0};
#endif

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

            int rawv = 0; // declared before, because its needed outside of the HAS VOLTAGE also

            // ---- Sample ----
#if HAS_VOLTAGE
            {
                float rawv = (float)analogRead(V_PIN);
                v_sum    += rawv;
                v_sum_sq += rawv * rawv;
            }
 #endif

            for (int i = 0; i < CT_COUNT; i++)
            {
                float rawi = (float)analogRead(CT_PINS[i]);
                ct_sum[i]    += rawi;
                ct_sum_sq[i] += rawi * rawi;
                ct_sum_p[i] += rawi *rawv;
            }
            
           

            n++;

            if (n >= (uint32_t)WINDOW_SAMPLES) // snapshot and queue
             {   
                NodeStatsSnapshot snap{};
                
                snap.t_ms = millis();

                for (int i = 0; i < CT_COUNT; i++)
                {
                    snap.ct[i].n      = n;   
                    snap.ct[i].sum    = ct_sum[i];
                    snap.ct[i].sum_sq = ct_sum_sq[i];
                    snap.ct[i].sum_p = ct_sum_p[i];
                }

#if HAS_VOLTAGE
                snap.v.n      = n;
                snap.v.sum    = v_sum;
                snap.v.sum_sq = v_sum_sq;
#endif 
                xQueueSend(q, &snap, 0);   // non-blocking

                // Reset window
                n = 0;
                for (int i = 0; i < CT_COUNT; i++)
                {
                    ct_sum[i] = 0.0;
                    ct_sum_sq[i] = 0.0;
                    ct_sum_p[i] = 0.0;
                }
#if HAS_VOLTAGE
                v_sum = 0.0;
                v_sum_sq = 0.0;
#endif
            }
            // Yield so other tasks can run
            taskYIELD();
        }
    }
}
