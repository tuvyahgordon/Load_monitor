#pragma once

#ifndef MQTT_BASE_TOPIC
#define MQTT_BASE_TOPIC "home/load_meter"
#endif

#ifndef NODE_ID
#define NODE_ID "node_unknown"
#endif

#ifndef CT_COUNT
#define CT_COUNT 1
#endif

#ifndef HAS_VOLTAGE
#define HAS_VOLTAGE 0
#endif

#ifndef SAMPLE_RATE_HZ
#define SAMPLE_RATE_HZ 4000
#endif

#ifndef WINDOW_SAMPLES
#define WINDOW_SAMPLES 4000
#endif

// Pins (defaults for IntelliSense)
#ifndef CT1_PIN
#define CT1_PIN 32
#endif

#ifndef CT2_PIN
#define CT2_PIN 35
#endif

#ifndef V_PIN
#define V_PIN 34
#endif

constexpr double MAINS_VRMS = 230;
 
//Isecondary= Iprimary/1000 -> 
//Vburden=Iprimary*Rburden -> Vburden=Iprimary*0.02 (assuming Rburden is 20ohm)
//Iprimary=50(adc*3.3)/4095
//=>
//Iprimary =adc*0.0403

constexpr double ADC_TO_AMPS = 0.0403; //Isecondary= Iprimary/1000 -> Vburden=Iprimary*Rburden -> Vburden=Iprimary*0.02 -> Iprimary= ADC counts = Vburden/3.3*4096, so ADC_TO_AMPS = 1000*3.3/(4096*Rburden). With Rburden=62ohm, this is about 0.0403 A per ADC count. Adjust Rburden for different CTs or to change the range/resolution.
// ADC properties
constexpr double ADC_REF = 3.3;
constexpr double ADC_COUNTS = 4096.0;

// Voltage divider (100k / 10k)
constexpr double VDIV_TOP = 100000.0;
constexpr double VDIV_BOTTOM = 10000.0;
constexpr double VDIV_GAIN = (VDIV_TOP + VDIV_BOTTOM) / VDIV_BOTTOM; // = 11

// AC adapter ratio (approximate)
constexpr double ACAC_RATIO = 230.0 / 9.0;  // adjust if needed

// Calibration factor (tune after measuring with a multimeter)
constexpr double V_CAL = 1.0;

// Final conversion: ADC counts → mains volts
constexpr double ADC_TO_VOLTS =
    (ADC_REF / ADC_COUNTS) * VDIV_GAIN * ACAC_RATIO * V_CAL;


    // Sanity checks
#if CT_COUNT < 1 || CT_COUNT > 2
#error "CT_COUNT must be 1 or 2"
#endif

#if HAS_VOLTAGE && !defined(V_PIN)
#error "HAS_VOLTAGE=1 but V_PIN not defined"
#endif
