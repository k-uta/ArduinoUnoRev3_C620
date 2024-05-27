#include <SPI.h>
#include "mcp_can.h"

// target angular velocity
float target_omega = 2*PI; // rad/sec

// M3508 constants
#define M3508_MAX_OUT 16384
#define M3508_GEAR_RATIO 3591.f / 187.f

const float m3508_angle_coef = 2.f * PI / (M3508_GEAR_RATIO * 256.f);
const float m3508_rpm_coef = 2.f * PI / (60.f * M3508_GEAR_RATIO);
const float m3508_output_coef = 20/M3508_MAX_OUT;

int16_t rotor_angle = 0;
int16_t rotor_rpm = 0;
int16_t rotor_torque = 0;

int8_t temperature;

/// @brief  angle of gearhead output shaft in radians.
float output_angle = 0.0f;
/// @brief angular velocity of gearhead output shaft in radians per sec.
float output_omega = 0.0f;
/// @brief motor current [A]
float output_current = 0.0f;

// PID constants
#define Kp 400.0
#define Ki 5.0
#define Kd 0.0
#define max_integral M3508_MAX_OUT // Maximum value for integral term (anti-windup)

// PID variables
float input = 0.0f;
float output = 0;
float integral = 0;
float previous_error = 0;
float previous_input = 0;
unsigned long last_time = 0;

// Control loop interval in milliseconds
#define control_interval 10 // 10 ms (100 Hz)

// MCP2515
#define SPI_CS_PIN 10
MCP_CAN CAN(SPI_CS_PIN); // Set CS pin
/*
  Pin Mapping
    MCP -> Arduino
      sck -> 13
      si -> 11
      so -> 12
      cs -> 10
*/

void setup() {
  Serial.begin(115200); 

  // Initialize MCP2515
  while (CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) != CAN_OK) {
    Serial.println("Error Initializing MCP2515...");
    delay(1000);
  }
  Serial.println("MCP2515 Initialized Successfully!");
  CAN.setMode(MCP_NORMAL);

  last_time = millis();
}

void loop() {
  unsigned long current_time = millis();
  if (current_time - last_time >= control_interval) {
    last_time = current_time;

    // Read current RPM from motor controller
    byte len = 0;
    unsigned char buf[8];
    long unsigned int rxId;
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
      CAN.readMsgBuf(&rxId, &len, buf);

      if (rxId == 0x202) { // Assuming ID 1 for the speed controller
        int16_t raw_angle = (buf[0] << 8) | buf[1];
        int16_t raw_rpm = (buf[2] << 8) | buf[3];
        int16_t raw_current = (buf[4] << 8) | buf[5];
        temperature = buf[6];

        int8_t diff = (uint8_t)(raw_angle >> 5) - (uint8_t)(rotor_angle >> 5);
        output_angle += static_cast<float>(diff) * m3508_angle_coef;
        rotor_angle = raw_angle;

        output_omega = raw_rpm * m3508_rpm_coef;
        output_current = raw_current * m3508_output_coef;

        // Print feedback to Serial Monitor
        Serial.print("Angle: ");
        Serial.print(output_angle);
        Serial.print("\tRPM: ");
        Serial.print(output_omega);
        Serial.print("\tCurrent: ");
        Serial.print(output_current);
        Serial.print("\tTemperature: ");
        Serial.print(temperature);
        Serial.print("\n");
      }
    }

    // PID calculations
    float error = target_omega - output_omega; // P
    integral += error * control_interval; // I
    float derivative = (error - previous_error) / control_interval; // D
    previous_error = error;

    // Anti-windup
    if (integral > max_integral) integral = max_integral;
    else if (integral < -max_integral) integral = -max_integral;

    // Reset integral term if motor is not running
    if (temperature == 0) integral = 0;

    output = Kp * error + Ki * integral + Kd * derivative;

    // Constrain output to max current range
    int16_t current_output = constrain((int16_t)output, -M3508_MAX_OUT, M3508_MAX_OUT);

    // Send control command to motor controller
    unsigned char current_data[8] = {(uint8_t)(0), 
                                     (uint8_t)(0),
                                     (uint8_t)((current_output & 0xff00) >> 8), 
                                     (uint8_t) (current_output & 0x00ff), 
                                     (uint8_t)(0), 
                                     (uint8_t)(0), 
                                     (uint8_t)(0), 
                                     (uint8_t)(0)};
    CAN.sendMsgBuf(0x200, 0, 8, current_data); // Sending to ID 1
  }
}
