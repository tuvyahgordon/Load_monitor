#include "Averager.h"
#include "Common.h"
#include <Arduino.h>
#include <math.h>

// This task receives StatsSnapshot packets from the sampler queue,
// computes mean + RMS (+ variance if you want), and (for now) prints them.
void averagerTask(void* arg)
{
auto* a = (AveragerArgs*)arg; // q_in 
// Safety: if nothing is passed to the queue
if (!a || !a->q_in || !a->q_out) { for(;;) vTaskDelay(pdMS_TO_TICKS(1000)); }

StatsSnapshot snap{};

for (;;)
{
    // Block here until a snapshot arrives
    if (xQueueReceive(a->q_in, &snap, portMAX_DELAY) == pdTRUE)
    {
        // Guard against divide-by-zero
        if (snap.n == 0) continue;

        // Mean of raw ADC codes (0..4095)
        const double mean = snap.sum / (double)snap.n;
        const double mean_sq = snap.sum_sq / (double)snap.n;
        double ac_power = mean_sq - mean*mean;
        if (ac_power<0) ac_power = 0;
        const double rms = sqrt(ac_power);
        
        MetricsSnapshot m{};
        m.n = snap.n;
        m.mean = mean;
        m.rms = rms;
        m.t_ms = millis(); 

        xQueueSend(a->q_out, &m, 0);
        
        // For now: show results in Serial Monitor
        // (to remove this later and send to Pi instead)
        Serial.print("n=");
        Serial.print(snap.n);
        Serial.print("  mean=");
        Serial.print(mean, 3);
        Serial.print("  rms=");
        Serial.print(rms, 3);
    }
}
}
