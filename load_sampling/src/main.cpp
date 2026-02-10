 #include <Arduino.h>
 #include "Sampler.h"
 #include "Averager.h"
 #include "Publisher.h"
 #include "Common.h"


 void setup() {
  QueueHandle_t statsQueue = xQueueCreate(4, sizeof(StatsSnapshot));
  QueueHandle_t metricsQueue = xQueueCreate(4, sizeof(MetricsSnapshot));
  static AveragerArgs avgArgs;
    avgArgs.q_in = statsQueue;
    avgArgs.q_out = metricsQueue;


 xTaskCreatePinnedToCore(samplerTask, "sampler", 4096, (void*)statsQueue, 3, NULL, 1);
 xTaskCreatePinnedToCore(averagerTask, "averager", 4096, &avgArgs, 2, NULL, 1);
 xTaskCreatePinnedToCore(publisherTask,"publisher", 6144,(void*)metricsQueue, 1, NULL, 0);
 
}

void loop() {
}
