#include <NewPing.h>
#include "Timer.h"

// SONAR

const int sonar_vcc = 11;
const int sonar_trig = 12;
const int sonar_echo = 13;
const int sonar_max_distance = 100;
const int max_distance = 100;
const int minimum_distance = 10;

// LED

const int yellow_LED_p = 2;
const int yellow_LED_n = 3;

// VALVE

const int motor_pin_p = 10;
const int motor_pin_n = 9;

// BUTTON

const int button_in = 8;
int button_state = 0;

// TIMER

Timer timer_print(MILLIS);
Timer timer_calculate(MILLIS);
const int print_timeout = 5000;
const int calculate_timeout = 1000;

// COMMON VAR

const int arr_ammount = 50;
int jmlh_air_avg_global = 50;
int is_open = false;
int bypass = false;
int percentage_i = 0;
float distance;

int arr_percentage[arr_ammount];
NewPing sonar(sonar_trig, sonar_echo, sonar_max_distance);

//////////////////////////////// INIT ////////////////////////////////

void setup() {

  setup_pin();
  do_blink_led(3, 500);

  timer_print.start();
  timer_calculate.start();

  Serial.begin(9600);
}

//////////////////////////////// MAIN LOOP ////////////////////////////////

void loop() {

  // Sonar thing

  distance = min(sonar.ping_cm(), max_distance);
  distance = max(5.0, distance);

  // Check bypass

  bypass = digitalRead(button_in) ? false : true;

  // Check the water level

  if (jmlh_air_avg_global <= 60 || bypass){
    is_open = true;
  }
  if (jmlh_air_avg_global >= 100){
    is_open = false;
    bypass = false;
  }

  // Percentages array

  int jmlh_air = 100 - ((distance - 10) / max_distance * 100);
  
  if (percentage_i < arr_ammount){
    arr_percentage[percentage_i] = jmlh_air;
  }
  else {
    percentage_i = 0;
  }
  percentage_i += 1;

  // Average Water Calculator

  if (timer_calculate.read() >= calculate_timeout){
    timer_calculate.stop();
    timer_calculate.start();
    int jmlh_air_avg = 0;
    for (int i = 0; i < arr_ammount; i++){
      if (arr_percentage[i] != 0){
        jmlh_air_avg += arr_percentage[i];
      }
    }
    jmlh_air_avg_global = jmlh_air_avg/arr_ammount;
  }

  // Display something

  if (timer_print.read() >= print_timeout){
    timer_print.stop();
    timer_print.start();

    Serial.print("Keran: ");
    Serial.print(is_open ? "Terbuka" : "Tertutup");
    Serial.print(" ; Jumlah Air: ");
    Serial.print(min(jmlh_air_avg_global, 100));
    Serial.println("%");
  }

  // Toggle mechine
  
  set_valve(is_open);
  set_led(is_open);

}

//////////////////////////////// FUNCTIONS ////////////////////////////////

void setup_pin(){
  const int pins_out[] = { motor_pin_p, motor_pin_n, yellow_LED_p, yellow_LED_n, sonar_vcc };
  const int pins_in[] = { sonar_trig, sonar_echo };
  const int pins_out_size = sizeof(pins_out)/sizeof(int);
  const int pins_in_size = sizeof(pins_in)/sizeof(int);

  for (int i = 0; i < pins_out_size; i++){
    pinMode(pins_out[i], OUTPUT);
  }

  for (int i = 0; i < pins_in_size; i++){
    pinMode(pins_in[i], INPUT);
  } 

  pinMode(button_in, INPUT_PULLUP);
  digitalWrite(sonar_vcc, HIGH);
}

void do_blink_led(int n_times, int delays){
  // Blink LED n times before starting
  int set = false;
  for (int i = 1; i <= (n_times*2) + 1; i++){
    set_led(set);
    set = !set;
    delay(delays);
  }
}

void set_valve(bool is_active){
  digitalWrite(motor_pin_p, is_active ? HIGH : LOW);
  digitalWrite(motor_pin_n, LOW);
}

void set_led(bool is_active){
  digitalWrite(yellow_LED_p, is_active ? HIGH : LOW);
  digitalWrite(yellow_LED_n, LOW);
}
