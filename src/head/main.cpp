#include <Arduino_Alvik.h>
#include <array>
#include <cmath>
#include <WiFi.h>
#include <ArduinoWebsockets.h>

using namespace websockets;

const char* ssid = "roomba";
const char* password = "12345678";

WebsocketsServer wsServer;
WebsocketsClient client;

unsigned long lastSend = 0;
int counter = 0;

Arduino_Alvik alvik;

uint8_t* distance_map = nullptr;

float points[64][3];
float pos[2];
float rot;

int last_reconnect = millis();

void setup() {
  alvik.begin();

  Serial.begin(115200);

  delay(1000);

  WiFi.softAPConfig(IPAddress(10,0,0,1), IPAddress(10,0,0,1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);

  wsServer.listen(8080);
  Serial.println("WebSocket server started on port 8080");

  delay(5000);
}

void convert_tof_reading_to_obj_space() {
  for(int i = 0; i < 64; ++i) {
    float yaw = (30 - (60/8 * (i%8))) * (M_PI / 180.0);
    float pitch = (30 - (60/8 * (i/8))) * (M_PI / 180.0);

    float reading = distance_map[i] * 10.0; // convert to mm
    points[i][0] = reading;
    points[i][1] = reading * std::tan(yaw);
    points[i][2] = reading * std::tan(pitch);
  }
}

void convcert_obj_space_to_world_space() {
  float cos_rot = std::cos(rot * (M_PI / 180.0));
  float sin_rot = std::sin(rot * (M_PI / 180.0));

  for(int i = 0; i < 64; ++i) {
    float x_obj = points[i][0];
    float y_obj = points[i][1];

    float worldX = x_obj * cos_rot - y_obj * sin_rot + pos[0] * 10;
    float worldY = x_obj * sin_rot + y_obj * cos_rot + pos[1] * 10;

    points[i][0] = ((int)(worldX * 100.0))/100.0;
    points[i][1] = ((int)(worldY * 100.0))/100.0;
  }
}

int last_measure = 0;


const float FORWARD_SPEED = 12.0;   // cm/s
const float TURN_ANGLE    = 25.0;   // degrees
const int   SAFE_DISTANCE = 25;     // cm

float left, centerLeft, center, right, centerRight;

int scanDirection = 1;

void loop() {

  if(millis() - last_measure > 15000 || millis() < 15000) {
    Serial.println("Start scanning");
    delay(100);
    alvik.brake();
    if (!client.available()) {
      client = wsServer.accept();
      client.setFragmentsPolicy(FragmentsPolicy_Aggregate);
      if (client.available()) {
        Serial.println("Client connected");
      }
    }

    for(int i = 0; i < 36; ++i) {
        alvik.rotate(12 * scanDirection);
        alvik.request_distance_map(5, 5000);
        distance_map = alvik.get_distance_map();
        convert_tof_reading_to_obj_space();
        convcert_obj_space_to_world_space();
        alvik.get_pose(pos[0], pos[1], rot);

        if (client.available()) {
          Serial.println("Sending");
            client.sendBinary((char*)points, sizeof(points));
        }
        delay(100);
      }
    last_measure = millis();

    scanDirection *= -1;
  }

  // Read the five distance zones
  alvik.get_distance(left, centerLeft, center, centerRight, right);

  if (center < SAFE_DISTANCE ||
      centerLeft < SAFE_DISTANCE ||
      centerRight < SAFE_DISTANCE) {

    alvik.brake();
    delay(100);

    // Turn toward the side with more free space
    if ((left + centerLeft) > (right + centerRight)) {
      alvik.rotate(TURN_ANGLE);   // turn left
    } else {
      alvik.rotate(-TURN_ANGLE);    // turn right
    }

    delay(100);
  }
  else {
    // Path is clear
    alvik.drive(FORWARD_SPEED, 0.0);
  }

  delay(50);

}

