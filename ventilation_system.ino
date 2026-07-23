// Adaptive Thermal Response Ventilation System

const int tempPin = A0;  // Arduino pin used for temperature sensor
const int fanPin = 6;    // Pin where the fan is connected

const float tempMin = 25.0;  // Minimum temperature threshold for fan activation
const float tempMax = 70.0;  // Maximum temperature threshold for max fan speed
const float hysteresis = 1.0; // Prevents rapid switching near thresholds

// Non-blocking timing variables
unsigned long previousMillis = 0;
const long interval = 1000;   // Interval at which to read sensor (ms)

class VentilationController {
  private:
    int pin;
    int currentSpeed;
    bool fanState;

  public:
    VentilationController(int controlPin) {
      pin = controlPin;
      currentSpeed = 0;
      fanState = false;
    }

    void begin() {
      pinMode(pin, OUTPUT);
    }

    void update(float temp) {
      if (!fanState && temp >= tempMin) {
        fanState = true; // Turn on when temp reaches minimum
      } else if (fanState && temp < (tempMin - hysteresis)) {
        fanState = false; // Turn off only when temp drops below minimum minus hysteresis
      }

      if (!fanState) {
        currentSpeed = 0;
        digitalWrite(pin, LOW);
      } else {
        if (temp >= tempMax) {
          currentSpeed = 255;
        } else {
          currentSpeed = map((long)(temp * 10), (long)(tempMin * 10), (long)(tempMax * 10), 32, 255);
        }
        analogWrite(pin, currentSpeed);
      }
    }

    int getSpeed() {
      return currentSpeed;
    }
};

VentilationController myVentilation(fanPin);

// Returns NAN if the sensor reading looks invalid
float readTemperature() {
  int rawValue = analogRead(tempPin);

  // TMP36: 750mV at 25C, 10mV/C, 500mV offset at 0C
  float voltage = rawValue * (5.0 / 1024.0);
  float celsius = (voltage - 0.5) * 100.0;

  // Basic sanity check to catch a disconnected/faulty sensor
  if (celsius < -40.0 || celsius > 125.0) {
    return NAN;
  }
  return celsius;
}

void setup() {
  pinMode(tempPin, INPUT);
  Serial.begin(9600);
  myVentilation.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // Non-blocking delay check
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float currentTemp = readTemperature();

    if (isnan(currentTemp)) {
      Serial.println("Sensor error: reading out of range");
      // Fail-safe: keep fan off rather than acting on bad data
    } else {
      Serial.print("Temperature: ");
      Serial.println(currentTemp);
      myVentilation.update(currentTemp);
    }
  }
}
