#include "esp_system.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== ESP32 Info ===");
  Serial.printf("Chip model: %s\n", esp_get_idf_version());
  Serial.printf("Chip revision: %d\n", ESP.getChipRevision());
  Serial.printf("CPU cores: %d\n", ESP.getChipCores());
  Serial.printf("CPU frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("Chip model: %s\n", ESP.getChipModel());
}

void loop() {}
