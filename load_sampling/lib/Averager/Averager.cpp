#include "Averager.h"
#include "Common.h"
#include "Config.h"
#include <Arduino.h>
#include <math.h>

// This task receives StatsSnapshot packets from the sampler queue,
// computes RMS, and (for now) prints them.
void averagerTask(void* arg)
{
auto* a = (AveragerArgs*)arg; // q_in 
// Safety: if nothing is passed to the queue
if (!a || !a->q_in || !a->q_out) { for(;;) vTaskDelay(pdMS_TO_TICKS(1000)); }

NodeStatsSnapshot statsnap{}; 

for (;;)
{
    // Block here until a snapshot arrives
    if (xQueueReceive(a->q_in, &statsnap, portMAX_DELAY) == pdTRUE)
    {
        NodeMetricsSnapshot m{}; // this is where the results will be stored
        m.t_ms = statsnap.t_ms;
        double vrms = 0.0;
        double vmean = 0.0;
#if HAS_VOLTAGE
        const auto& v = statsnap.v;
        if (v.n == 0) m.vrms = 0.0;
        else
        {
            const double vmean    = v.sum    / (double)v.n;
            const double vmean_sq = v.sum_sq / (double)v.n;
            double vrms_sq = vmean_sq - vmean * vmean;
            if (vrms_sq < 0.0) vrms_sq = 0.0;
            double vrms_adc = sqrt(vrms_sq);
            double vrms = vrms_adc * ADC_TO_VOLTS;
            m.vrms = vrms;
        }
#endif
        for (int i = 0; i < CT_COUNT; i++)
        {
        const auto& s = statsnap.ct[i];

        // Guard against divide-by-zero
        if (s.n == 0) { m.irms[i] = 0.0; continue; }
 
        // Mean of raw ADC codes (0..4095)
        const double imean = s.sum / (double)s.n;
        const double imean_sq = s.sum_sq / (double)s.n;
#if HAS_VOLTAGE
        const double p_mean = s.sum_p / (double)s.n;      
#endif
        
        double irms_sq = imean_sq - imean*imean; 
        if (irms_sq<0.0) irms_sq = 0.0;
        double irms_adc = sqrt(irms_sq);
        double irms = irms_adc * ADC_TO_AMPS;
        m.irms[i] = irms;
        m.apparpower[i] = irms * MAINS_VRMS; // calculate based on the given value for mains
#if HAS_VOLTAGE
        m.apparpower[i] = irms * vrms; // calculate with real vrms
        double mean_p = s.sum_p / s.n;
        double real_power_adc = mean_p - vmean * imean; // removes bias
        double real_power = real_power_adc * ADC_TO_VOLTS * ADC_TO_AMPS;
        m.power[i] = real_power;
#endif
        }
        xQueueSend(a->q_out, &m, 0);
        
        // For now: show results in Serial Monitor
        // (to remove this later and send to Pi instead)
        
        for (int i = 0; i < CT_COUNT; i++) 
        {
            Serial.print(" ct");
            Serial.print(i + 1);
            Serial.print("_rms=");
            Serial.print(m.irms[i], 3);
        }
#if HAS_VOLTAGE
        Serial.print(" vrms=");
        Serial.print(m.vrms, 3);
#endif
        Serial.println();
    }
}
}

