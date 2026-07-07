#include "Arduino_Alvik.h"

unsigned long baud = 115200;


static void onLineChange(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_base == ARDUINO_USB_CDC_EVENTS) {
    arduino_usb_cdc_event_data_t * data = (arduino_usb_cdc_event_data_t*)event_data;
    switch (event_id) {
      case ARDUINO_USB_CDC_LINE_CODING_EVENT:
        auto baud = data->line_coding.bit_rate;
        Serial0.updateBaudRate(baud);
        while (Serial0.available()) {
          Serial0.read();
        }
        break;
    }
  }
}

void setup() {
  Serial.onEvent(onLineChange);
  Serial.enableReboot(false);
  Serial.begin(baud);
  Serial.setRxBufferSize(0);
  Serial.setRxBufferSize(2048);
  Serial0.setRxBufferSize(8192);
  Serial0.setTxBufferSize(8192);
  Serial0.begin(baud, SERIAL_8E1);
  Serial0.flush();
  pinMode(BOOT_STM32, OUTPUT);
  pinMode(RESET_STM32, OUTPUT);
  digitalWrite(BOOT_STM32,HIGH);
  delay(1000);
  digitalWrite(RESET_STM32,LOW);
  delay(1000);
  digitalWrite(RESET_STM32,HIGH);
}

void loop() {
  int len = 0;
  uint8_t auc_buffer[488];
  while (Serial.available() && len < sizeof(auc_buffer)) {
    auc_buffer[len++] = Serial.read();
  }
  if (len) {
    Serial0.write(auc_buffer, len);
  }

  len = 0;
  while (Serial0.available() && len < sizeof(auc_buffer)) {
    auc_buffer[len++] = Serial0.read();
  }
  if (len) {
    Serial.write(auc_buffer, len);
  }
}