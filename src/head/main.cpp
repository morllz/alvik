#include <Arduino_Alvik.h>

Arduino_Alvik alvik;

float reference = 150.0;
float error = 0.0;
float distance[5];

float sign(float x) {
  return (x > 0) - (x < 0);
}

void setup(){
  alvik.begin();
  Serial.begin(115200);
}

void loop(){

  alvik.get_distance(distance[0], distance[1], distance[2], distance[3], distance[4], MM);

  float min = distance[0];
  int i = 0;
  for(int j = 1; j < 5; j++) {
    if(distance[j] < min) {
      min = distance[j];
      i = j;
    }
  }

  error = min - reference;

  if(i == 0) {
    alvik.set_wheels_speed((1-0.3*sign(error)) * error, (1+0.3*sign(error)) * error);
    return;
  }
  if(i == 4) {
    alvik.set_wheels_speed((1+0.3*sign(error)) * error, (1-0.3*sign(error)) * error);
    return;
  }

  alvik.set_wheels_speed(error, error);
}