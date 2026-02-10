#include "Publisher.h"
#include "Common.h"
#include "secrets.h"
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>


// -------------------- CONFIG --------------------

// Topic to publish to
static const char* MQTT_TOPIC = "home/energy/ct1";

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

void publisherTask(void* arg)
{
    QueueHandle_t q = (QueueHandle_t)arg;
    if (q == nullptr)
    {
        for (;;)
            vTaskDelay(pdMS_TO_TICKS(1000));
    }

    MetricsSnapshot m{};

    for (;;)
    {
        // Wait for metrics from averager
        if (xQueueReceive(q, &m, portMAX_DELAY) == pdTRUE)
        {
            ensure_wifi();
            ensure_mqtt();

            // Keep MQTT client alive
            mqtt.loop();

            // Build JSON payload
            // Note: snprintf with %.3f is supported on ESP32 (newlib)
            char payload[192];
            const unsigned long t_ms = (unsigned long)m.t_ms;
            const unsigned long n    = (unsigned long)m.n;

            snprintf(payload, sizeof(payload),
                     "{\"t_ms\":%lu,\"n\":%lu,\"mean\":%.3f,\"rms_ac\":%.3f}",
                     t_ms, n, m.mean, m.rms);

            // Publish
            mqtt.publish(MQTT_TOPIC, payload, MQTT_RETAIN);
        }
    }
}
