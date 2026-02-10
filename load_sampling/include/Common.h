#pragma once  
#include <stdint.h>
#include <Arduino.h>           
#include "freertos/FreeRTOS.h"    // defines BaseType_t, TickType_t, etc.
#include "freertos/queue.h"       // defines QueueHandle_t
struct StatsSnapshot
{
    uint32_t n;
    double   sum;
    double   sum_sq;
};

struct MetricsSnapshot 
{
    uint32_t n;
    double mean; // possibly take out later, if not needed
    double rms;     // unbiased/AC RMS
    double t_ms; // time of snapshot
};

struct AveragerArgs {
    QueueHandle_t q_in;   // StatsSnapshot from sampler
    QueueHandle_t q_out;  // MetricsSnapshot to publisher
};