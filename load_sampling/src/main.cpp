 #include <Arduino.h>
 #include "Sampler.h"
 #include "Averager.h"
 #include "Publisher.h"
 #include "Common.h"


 void setup() {
  QueueHandle_t statsQueue = xQueueCreate(4, sizeof(NodeStatsSnapshot));
  QueueHandle_t metricsQueue = xQueueCreate(4, sizeof(NodeMetricsSnapshot));
  static AveragerArgs avgArgs;
    avgArgs.q_in = statsQueue;
    avgArgs.q_out = metricsQueue;

  Serial.begin(115200);
  delay(200);
  Serial.println("BOOT: main setup running");

 xTaskCreatePinnedToCore(samplerTask, "sampler", 4096, (void*)statsQueue, 3, NULL, 1);
 xTaskCreatePinnedToCore(averagerTask, "averager", 6144, &avgArgs, 2, NULL, 1);
 xTaskCreatePinnedToCore(publisherTask,"publisher", 8192,(void*)metricsQueue, 1, NULL, 0);
 
}

void loop() {
}
