#include "Arduino_Alvik.h"

unsigned long baud = 115200;

int rts = -1;
int dtr = -1;


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
  Serial.setRxBufferSize(2048);
  Serial0.setRxBufferSize(8192);
  Serial0.setTxBufferSize(8192);
  Serial0.begin(baud, SERIAL_8E1);
  Serial0.flush();
  pinMode(BOOT_STM32, OUTPUT);
  pinMode(RESET_STM32, OUTPUT);
  digitalWrite(BOOT_STM32, HIGH);
  delay(10);
  digitalWrite(RESET_STM32, LOW);
  delay(20);
  digitalWrite(RESET_STM32, HIGH);
  delay(200);
}

void loop() {
  uint8_t buf[512];

  int n = Serial.available();
  if (n > 0) {
    n = Serial.readBytes(buf, min(n, (int)sizeof(buf)));
    Serial0.write(buf, n);
  }

  n = Serial0.available();
  if (n > 0) {
    n = Serial0.readBytes(buf, min(n, (int)sizeof(buf)));
    Serial.write(buf, n);
  }

  delay(1);
}