#include "Publisher.h"
#include "Common.h"
#include "secrets.h"
#include "Config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>


// -------------------- CONFIG --------------------

// Optional: to set if the broker requires auth
static const char* MQTT_USER = nullptr;  // "user"
static const char* MQTT_PASS = nullptr;  // "pass"

// Keep last value on broker (useful for Grafana)
static const bool MQTT_RETAIN = true;

// ------------------------------------------------

static WiFiClient wifiClient;
static PubSubClient mqtt(wifiClient);

static void ensure_wifi()
{
    if (WiFi.status() == WL_CONNECTED) return;

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Block until connected (publisher is allowed to block)
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

static void sync_time()
{
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    time_t now = time(nullptr);
    while (now < 100000)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        now = time(nullptr);
    }
}

static void ensure_mqtt()
{
    mqtt.setServer(MQTT_HOST, MQTT_PORT);

    while (!mqtt.connected())
    {
        // Unique-ish client ID
        String clientId = "esp32-pub-";
        clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

        bool ok = false;
        if (MQTT_USER && MQTT_PASS)
            ok = mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS);
        else
            ok = mqtt.connect(clientId.c_str());

        if (!ok)
        {
            // pause before retrying
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

static char mqtt_topic[128];

static const char* buildTopic(const char* suffix)
{
    // e.g. "home/load_meter/node_main/metrics"
    snprintf(mqtt_topic, sizeof(mqtt_topic), "%s/%s/%s", MQTT_BASE_TOPIC, NODE_ID, suffix);
    return mqtt_topic;
}

void publisherTask(void* arg)
{
    QueueHandle_t q = (QueueHandle_t)arg;
    if (q == nullptr)
    {
        for (;;)
            vTaskDelay(pdMS_TO_TICKS(1000));
    }

    NodeMetricsSnapshot m{};

    for (;;)
    {
        // Wait for metrics from averager
        if (xQueueReceive(q, &m, portMAX_DELAY) == pdTRUE)
        {
            ensure_wifi();
            ensure_mqtt();
            sync_time();
            // Keep MQTT client alive
            mqtt.loop();

            // Build JSON payload
            // Note: snprintf with %.3f is supported on ESP32 (newlib)
            char payload[512];

            int len = 0;
            time_t now = time(nullptr);

            int w = snprintf(payload + len, sizeof(payload) - len,
                "{\"node\":\"%s\",\"epoch\":%lu,\"t_ms\":%lu",
                NODE_ID,
                (unsigned long)now,
                (unsigned long)m.t_ms);
                     
                 // add CT channels (1 or 2)
            for (int i = 0; i < CT_COUNT; i++)
            {
                w = snprintf(payload + len, sizeof(payload) - len,
                    ",\"ct%d_irms\":%.3f,\"ct%d_apparpower\":%.3f",
                    i + 1,
                    m.irms[i],
                    i + 1,
                    m.apparpower[i]);
                if (w < 0 || w >= (int)(sizeof(payload) - len)) { Serial.println("JSON overflow!"); continue; }
                len += w;

                 // add voltage only if it exists
#if HAS_VOLTAGE
                    w = snprintf(payload + len, sizeof(payload) - len,
                    ",\"ct%d_power\":%.3f",
                    i + 1, m.power[i]);
                    if (w < 0 || w >= (int)(sizeof(payload) - len)) { Serial.println("JSON overflow!"); continue; }
                    len += w;
#endif

            }
#if HAS_VOLTAGE
// add rms voltage ONCE
            w = snprintf(payload + len, sizeof(payload) - len,
            ",\"vrms\":%.3f", m.vrms);
            if (w < 0 || w >= (int)(sizeof(payload) - len)) { Serial.println("JSON overflow!"); continue; }
            len += w;
#endif
// close JSON
            // check for overflow
            w = snprintf(payload + len, sizeof(payload) - len, "}");    
            if (w < 0 || w >= (int)(sizeof(payload) - len)) { Serial.println("JSON overflow!"); continue; }
            len += w;
        // Publish to MQTT
        mqtt.publish(buildTopic("metrics"), payload, MQTT_RETAIN);
        }
    }
}
