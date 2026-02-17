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
#define CT1_PIN 34
#endif

#ifndef CT2_PIN
#define CT2_PIN 35
#endif

#ifndef V_PIN
#define V_PIN 32
#endif

constexpr double MAINS_VRMS = 230;
constexpr double ADC_TO_AMPS = 0.2; //to change, depending on the CT sensor and burden resistor used. 
constexpr double ADC_TO_VOLTS = 3.3 / 4096.0; // 3.3V full scale, 12-bit ADC;

// Sanity checks
#if CT_COUNT < 1 || CT_COUNT > 2
#error "CT_COUNT must be 1 or 2"
#endif

#if HAS_VOLTAGE && !defined(V_PIN)
#error "HAS_VOLTAGE=1 but V_PIN not defined"
#endif
