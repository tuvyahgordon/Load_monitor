#pragma once  
#include <stdint.h>
#include <Arduino.h>           
#include "freertos/FreeRTOS.h"    // defines BaseType_t, TickType_t, etc.
#include "freertos/queue.h"       // defines QueueHandle_t
#include "config.h"

// Max supported in code 
static constexpr int MAX_CT = 2;

struct SignalSnapshot
{
    uint32_t n;
    double   sum;
    double   sum_sq;
};

struct PowerChannelSnapshot : SignalSnapshot
{
    float    sum_p;   // sum of v*i
};
struct NodeStatsSnapshot
{
    uint32_t t_ms;                 // end-of-window timestamp (millis)
    PowerChannelSnapshot ct[MAX_CT];      // if no voltage, will leave sum_p empty
#if HAS_VOLTAGE
    SignalSnapshot v;               // voltage channel stats
#endif
};

struct NodeMetricsSnapshot 
{
     uint32_t  t_ms; // time of snapshot
    double irms[MAX_CT];     // unbiased/AC RMS
    double apparpower[MAX_CT]; // calculated with given mains, or V if sampled
#if HAS_VOLTAGE 
    double vrms;
    double power[MAX_CT];  // calculated with V*I
#endif
};

struct AveragerArgs {
    QueueHandle_t q_in;   // StatsSnapshot from sampler
    QueueHandle_t q_out;  // MetricsSnapshot to publisher
};