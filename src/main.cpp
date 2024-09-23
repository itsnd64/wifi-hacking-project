#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <esp_task_wdt.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <vector>
#include <map>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
struct SSIDInfo {
    String ssid;
    std::array<uint8_t, 6> bssid;
    int m1_count;
    int m2_count;
    int m3_count;
    int m4_count;
};
std::map<uint8_t, std::vector<SSIDInfo>> ssidInfoMap;
std::vector<uint8_t> channels;

void scanAPs();

void setup() {
  esp_task_wdt_init(6000, false);
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.display();
  nvs_flash_init();
  esp_netif_init();
  esp_event_loop_create_default();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
}

void loop() {
}

void scanAPs() {
    esp_wifi_set_promiscuous(false);
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    uint16_t ap_num = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_num));
    wifi_ap_record_t ap_records[ap_num];
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));
    Serial.printf("Total APs found: %d\n", ap_num);

    channels.clear();
    ssidInfoMap.clear();
    display.clearDisplay();
    display.setCursor(0, 0);

    for (int i = 0; i < ap_num; i++) {
        wifi_ap_record_t ap = ap_records[i];
        Serial.printf("SSID: %s, Channel: %d\n", ap.ssid, ap.primary);
        display.printf("Ch%d|%d|%s\n", ap.primary, ap.rssi, ap.ssid);

        if (std::find(channels.begin(), channels.end(), ap.primary) == channels.end()) {
            channels.push_back(ap.primary);
        }

        SSIDInfo info;
        info.ssid = (char*)ap.ssid;
        memcpy(info.bssid.data(), ap.bssid, 6);
        info.m1_count = 0;
        info.m2_count = 0;
        info.m3_count = 0;
        info.m4_count = 0;
        ssidInfoMap[ap.primary].push_back(info);
    }
    if (channels.empty()) {
        for (int i = 1; i <= 13; i++) channels.push_back(i);
        display.println("No AP Found.");
    }
    display.display();
    esp_wifi_set_promiscuous(true);
}