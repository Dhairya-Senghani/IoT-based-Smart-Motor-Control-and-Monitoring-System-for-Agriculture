#include "BluetoothSerial.h"  // Include the BluetoothSerial library

// Create a Bluetooth Serial object
BluetoothSerial SerialBT;


#define PIN_VOLTAGE_R_Y 35

#define PIN_CURRENT_R 36
#define PIN_CURRENT_Y 39
#define PIN_CURRENT_B 34

#if ADC_RESOLUTION_BITS <= 8
typedef uint8_t VOLTAGE_VARIABLE_TYPE;
typedef uint8_t CURRENT_VARIABLE_TYPE;
#else
typedef uint16_t VOLTAGE_VARIABLE_TYPE;
typedef uint16_t CURRENT_VARIABLE_TYPE;
#endif

#define START_TO_DELTA_DELAY 5000

#define PIN_MOTOR_STARTING 13
#define PIN_MOTOR_RUNNING 12

// Default threshold value
VOLTAGE_VARIABLE_TYPE voltage_R_Y_threshold = 4095;

CURRENT_VARIABLE_TYPE current_R_threshold = 4095;
CURRENT_VARIABLE_TYPE current_Y_threshold = 4095;
CURRENT_VARIABLE_TYPE current_B_threshold = 4095;

VOLTAGE_VARIABLE_TYPE voltage_R_Y;

CURRENT_VARIABLE_TYPE current_R;
CURRENT_VARIABLE_TYPE current_Y;
CURRENT_VARIABLE_TYPE current_B;


String receivedData = "";  // Variable to accumulate the received data

int values[4];       // Array to store the four values
int valueIndex = 0;  // Index for tracking the received values
bool motor = 0;

void check_voltage();
void check_current();
void over_voltage();
void over_current();
void motor_ON();
void motor_OFF();

void setup() {
  pinMode(PIN_MOTOR_STARTING, OUTPUT);
  pinMode(PIN_MOTOR_RUNNING, OUTPUT);

  digitalWrite(PIN_MOTOR_STARTING, LOW);
  digitalWrite(PIN_MOTOR_RUNNING, LOW);

  Serial.begin(9600);
  
  if (!SerialBT.begin("ESP32_BT")) {
    Serial.println("An error occurred initializing Bluetooth");
  } else {
    Serial.println("Bluetooth initialized successfully. Now you can pair it.");
  }
}

void loop() {
  voltage_R_Y = analogRead(PIN_VOLTAGE_R_Y);
  current_R = analogRead(PIN_CURRENT_R);
  current_Y = analogRead(PIN_CURRENT_Y);
  current_B = analogRead(PIN_CURRENT_B);

  // Check if data is available via Bluetooth
  if (SerialBT.available()) {
    char incomingChar = SerialBT.read();  // Read the incoming character

    if (incomingChar == 'a') {
      check_voltage();
      Serial.println("turned ON motor");
    }

    else if (incomingChar == 'b') {
      motor_OFF();
      Serial.println("turned OFF motor");
    }

    else if (incomingChar == ',') {
      // When a comma is detected, store the accumulated value in the array
      switch (valueIndex) {
        case 0:
          current_R_threshold = ((receivedData.toInt() + 98) * 28);
          break;
        case 1:
          current_Y_threshold = ((receivedData.toInt() + 98) * 28);
          break;
        case 2:
          current_B_threshold = ((receivedData.toInt() + 98) * 28);
          break;
        case 3:
          voltage_R_Y_threshold = (receivedData.toInt() + 2850);
      }

      receivedData = "";  // Clear the receivedData string for the next value
      valueIndex++;       // Move to the next index for storing the next value

      // Reset valueIndex if all 4 values have been received
      if (valueIndex >= 4) {
        valueIndex = 0;
      }
    } else {
      // Append incoming characters (until a comma is found)
      receivedData += incomingChar;
    }
  }

  // Send the values as a single comma-separated string
  String message = String(current_R / 28 - 98) + "," + String(current_Y / 28 - 98) + "," + String(current_B / 28 - 98) + "," + String(voltage_R_Y - 2850);

  // Send the message via Bluetooth
  SerialBT.println(message);

  // Print to the Serial Monitor (for debugging purposes)
  Serial.println("Actual Values");
  Serial.print("R_Y_Volt: ");
  Serial.print(voltage_R_Y);
  Serial.print(" || R_Amp: ");
  Serial.print(current_R);
  Serial.print(" || Y_Amp: ");
  Serial.print(current_Y);
  Serial.print(" || B_Amp: ");
  Serial.println(current_B);

  Serial.println("Threshold values");
  Serial.print("R_Y_Volt: ");
  Serial.print(voltage_R_Y_threshold);
  Serial.print(" || R_Amp: ");
  Serial.print(current_R_threshold);
  Serial.print(" || Y_Amp: ");
  Serial.print(current_Y_threshold);
  Serial.print(" || B_Amp: ");
  Serial.println(current_B_threshold);

  check_current();
}




void check_voltage() {
  if (voltage_R_Y > voltage_R_Y_threshold) {
    over_voltage();
    motor_OFF();
  } else {
    motor_ON();
  }
}

void check_current() {
  if ((current_R > current_R_threshold) || (current_Y > current_Y_threshold) || (current_B > current_B_threshold)) {
    over_current();
  }
}

void over_voltage() {
  Serial.println("Over Voltage");
}

void over_current() {
  motor_OFF();
  Serial.println("Over Current");
}

void motor_ON() {
  digitalWrite(PIN_MOTOR_RUNNING, HIGH);
  digitalWrite(PIN_MOTOR_STARTING, HIGH);
  delay(START_TO_DELTA_DELAY / 5);
  check_current();
  delay(START_TO_DELTA_DELAY / 5);
  check_current();
  delay(START_TO_DELTA_DELAY / 5);
  check_current();
  delay(START_TO_DELTA_DELAY / 5);
  check_current();
  delay(START_TO_DELTA_DELAY / 5);
  check_current();
  digitalWrite(PIN_MOTOR_STARTING, LOW);
  motor = 1;
  Serial.println("Motor Strated");
}

void motor_OFF() {
  digitalWrite(PIN_MOTOR_RUNNING, LOW);
  digitalWrite(PIN_MOTOR_STARTING, LOW);
  Serial.println("Motor Stopped");
  motor = 0;
}