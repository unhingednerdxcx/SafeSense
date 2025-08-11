#include <Wire.h>
#include <WiFi.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <Arduino.h>
#include <driver/ledc.h>


hd44780_I2Cexp lcd;
WiFiClient client;

float temp_ana;
float temp_volt;
float temp;
float gas_volt;
float Rs_gas;
float ratio;
float Ro = 10.0;
float ppm;
int volume; // analog mic sensor: change to float if needed
long duration;
float distance;

const char* code = "::Ad3DaN/AJ2xm/wQ922/KdD::";
String data_sent;
String v_type;
const char* MQ = "8"; // change to your MQ sensor type
String response;
float b = -0.42, m = -0.36;
const char* ssid = "PUT NAME/SSID IN HERE";  // <--- Adjust this
const char* password = "PUT PASSWORD IN HERE"; // <--- Adjust this

const char* host = "XXX.XXX.XX.XX";  // ← your PC IP
const uint16_t port = 12245;
bool core_0_task = false;
bool core_1_task = false;

// Pins - FOLLOW README.MD wiring instructions
int temp_pin = 34; 
int gas_pin = 35;
int mphone_pin = 32;
int trig_pin = 5;
int echo_pin = 13;
int buzz_pin = 4;
// Codes struct
struct Codes {
  static constexpr const char* Accept = "111";
  static constexpr const char* Deny = "000";
  static constexpr const char* Pair = "120";
  static constexpr const char* Unpair = "301";
  static constexpr const char* Error = "101";
  static constexpr const char* Warning = "014";
  static constexpr const char* Reconnect = "211";
};

// funcs
String check_code(String par) { // Humanize codes
  if (par == Codes::Accept) return "Accept";
  else if (par == Codes::Deny) return "Deny";
  else if (par == Codes::Pair) return "Pair";
  else if (par == Codes::Unpair) return "Unpair";
  else if (par == Codes::Error) return "Error";
  else if (par == Codes::Warning) return "Warning";
  else if (par == Codes::Reconnect) return "Reconnect";
  else return "Error_Faced";
}

void get_data() { // Collect sensor values
  // === temp ===
  int temp_ana = analogRead(temp_pin);
  float temp_volt = temp_ana * (3.3 / 4095.0);
  float temp_c = (temp_volt - 0.5) * 100;
  
  // === Gas ===
  gas_volt = analogRead(gas_pin);
  gas_volt = gas_volt * (3.3 / 4095.0);
  if (gas_volt > 0.0) {
    Rs_gas = (5.0 - gas_volt) * 10.0 / gas_volt;
    ratio = Rs_gas / Ro;
    ppm = pow(10, ((log10(ratio) - b) / m));
  } 
  else {
    ppm = 0;
  }


  // === Sound ===
  volume = digitalRead(mphone_pin);
  v_type = "ANA/IO"; // change to "ANA" if using analog sensor

  // === Ultrasonic ===
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);

  duration = pulseIn(echo_pin, HIGH);
  distance = duration * 0.034 / 2;

  /*  IF YOU WANT TO TEST BEFORE PLUGGING IN ANY SENSORS, UNCOMMENT THIS PART
  temp = random(40);
  ppm = random(300);
  volume = random(234);
  distance = random(2011);
  */
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void send_data(void *pvParameters) {
  String request;
  int core = xPortGetCoreID();
  while (1) {
    get_data();
    data_sent = String(code) + " : " + core + " : " + temp + " : " + volume + " : " + v_type + " : " + distance + " : " + ppm + " : " + MQ + " : void";
    request = String(code) + " : rq" + " : " + Codes::Pair;

    unsigned long startTime = millis();
    unsigned long timeout = 20000; // 20 seconds
    bool timedOut = false;

    if (client.connect(host, port)) {
      Serial.println("Connected to pyth");
      client.println(request);
      vTaskDelay(1000);
      while (client.readStringUntil('\n') != Codes::Accept) {}
      vTaskDelay(1000);
      client.println(data_sent);
      Serial.println(data_sent);
      core_0_task = true;
      while (core_1_task == false) {}
      core_0_task = false;
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void hardware_control(void *pvParameters) {
  while (1) {
    while (core_0_task != true) {}
    if (temp <= 10 || temp >= 30) {
      lcd.setCursor(0, 0);
      lcd.print("!Warning!");
      lcd.setCursor(0, 1);
      lcd.print("Temp is high/low");
      vTaskDelay(1500);

      if (temp >= 0) {
        float pitch = temp * 26;
        ledcWriteTone(buzz_pin, pitch);
      } else {
        float pitch = (temp * 96) * -1;
        ledcWriteTone(buzz_pin, pitch);
      }
      vTaskDelay(1000);
      ledcWriteTone(buzz_pin, 0);
      lcd.clear();
    }
    else if (v_type != "I/O") {
      if (volume >= 60) {
        lcd.setCursor(0, 0);
        lcd.print("!Warning!");
        lcd.setCursor(0, 1);
        lcd.print("Too much noise");
        vTaskDelay(1500);
      }
      ledcWriteTone(buzz_pin, 0);
    }
    else if (v_type == "I/O") {
      if (volume == 1) {
        lcd.setCursor(0, 0);
        lcd.print("!Warning!");
        lcd.setCursor(0, 1);
        lcd.print("Too much noise");
        vTaskDelay(1500);
      }
      ledcWriteTone(buzz_pin, 0);
      lcd.clear();
    }
    else if (distance <= 400) {
      lcd.setCursor(0, 0);
      lcd.print("!Warning!");
      lcd.setCursor(0, 1);
      lcd.print("Object near");
      vTaskDelay(1500);

      float pitch = (850.0 / distance) * 100;
      ledcWriteTone(buzz_pin, pitch);
      vTaskDelay(1000);
      lcd.clear();
    }
    else if (ppm >= 500) {
      lcd.setCursor(0, 0);
      lcd.print("!Warning!");
      lcd.setCursor(0, 1);
      lcd.print("Gas Detected");
      vTaskDelay(1500);

      float pitch = 1.8 * distance;
      ledcWriteTone(buzz_pin, pitch);
    }
    else {
      ledcWriteTone(buzz_pin, 0);
      lcd.clear();
    }
    core_1_task = true;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// main
void setup() {
  Serial.begin(115200);
  while(!Serial);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  ledcAttach(buzz_pin, 1000, 8);
  vTaskDelay(2000);
  Wire.begin();
  lcd.begin(16, 2);
  lcd.setBacklight(true);
  pinMode(trig_pin, OUTPUT);
  pinMode(echo_pin, INPUT);
  pinMode(mphone_pin, INPUT);

  xTaskCreatePinnedToCore(send_data, "Sending data", 3000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(hardware_control, "handeling data in harware terms", 3000, NULL, 1, NULL, 1); 
}

void loop() {
  // using freeRTOS, keeping empty
}
