#include <avr/wdt.h>
#include <NewPing.h>

#define DEBUG 1

#define PIN_TRIGGER   2   // Trigger pin of ultrasonic sensor (can be same as echo)
#define PIN_ECHO      2   // Echo pin of ultrasonic sensor (can be same as trigger)

#define SENSOR2       1   // Enable 2nd sensor (for 2 bathrooms sharing an exhaust) 

#if SENSOR2
#define PIN_TRIGGER2  3   // Trigger pin of 2nd ultrasonic sensor (can be same as 2nd echo)
#define PIN_ECHO2     3   // Echo pin of 2nd ultrasonic sensor (can be same as 2nd trigger)
#endif

#define PIN_RELAY     4   // Pin to control exhaust relay
#define RELAY_ON      0   // 0 for active-low, 1 for active-high

#define THRESHOLD     20  // Threshold distance (in inches)

#define DELAY_ON    05000 // Delay before turning on exhaust (in milliseconds)
#define DELAY_OFF   60000 // Delay before turning off exhaust (in milliseconds)

#define DELAY_REFRESH 250 // Delay between sensor scans (in milliseconds, min 29)
#define NUM_READINGS  5   // Number of readings to average per scan

bool on = !RELAY_ON;      // Current state of exhaust (start with off)
unsigned long onTime;     // On delay start time
unsigned long offTime;    // Off delay start time
NewPing sonar(PIN_TRIGGER, PIN_ECHO);

#if SENSOR2
bool on2 = !RELAY_ON;
unsigned long onTime2;
unsigned long offTime2;
NewPing sonar2(PIN_TRIGGER2, PIN_ECHO2);
#endif

void setup() {

  // Turn off relay as soon as possible (for active-low)
  // This won't (shouldn't) make a difference for active-high relays
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, on ? RELAY_ON : !RELAY_ON);

#if DEBUG
  Serial.begin(9600);
#endif

#if DEBUG
  Serial.println(" -------------------------- ");
  Serial.println("| AutomaticExhaust v1.0    |");
  Serial.println("| written by Kevin Abraham |");
  Serial.println(" -------------------------- ");
  Serial.println();
  Serial.println("Source available at https://github.com/abraha2d/AutomaticExhaust");
  Serial.println();
#endif

  // Enable the watchdog with a 2 second timeout
  wdt_enable(WDTO_2S);

  for (int i = 0; i < 4; i++) {
    wdt_reset();
    digitalWrite(LED_BUILTIN, HIGH); delay(125);
    digitalWrite(LED_BUILTIN, LOW); delay(125);
  }

}

void loop() {

  wdt_reset();
  delay(DELAY_REFRESH);

  unsigned int distance = sonar.convert_in(sonar.ping_median(NUM_READINGS));

  // Light internal LED based on current sonar distance
  if (distance == 0) {
  } else if (distance > THRESHOLD) {
    digitalWrite(LED_BUILTIN, LOW);
  } else if (distance < THRESHOLD) {
    digitalWrite(LED_BUILTIN, HIGH);
  }

#if DEBUG
  Serial.print("Ping1: ");
  Serial.print(distance);
  Serial.print("in -> ");
#endif

  // If distance is 0, it's most likely a faulty measurement
  if (distance == 0) {

#if DEBUG
    Serial.print("Ignoring...");
#endif

  // If distance does not match current state (i.e. distance measurement just
  // crossed threshold), commence waiting
  } else if ((!on && distance > THRESHOLD) || (on && distance < THRESHOLD)) {
    onTime = offTime = millis();

#if DEBUG
    Serial.print("Stable: ");
    Serial.print(on ? "On" : "Off");
#endif

  // If distance is less than threshold, and we've waited more than DELAY_ON,
  // turn on the relay
  } else if (!on && distance < THRESHOLD && millis() - onTime > DELAY_ON) {
    on = true;

#if DEBUG
    Serial.print("Turning on...");
  } else if (!on && distance < THRESHOLD) {
    Serial.print("Turning on, ");
    Serial.print(DELAY_ON - (millis() - onTime));
    Serial.print("ms remaining");
#endif

  // If distance is greater than the threshold, and we've waited more than
  // DELAY_OFF, turn off the relay
  } else if (on && distance > THRESHOLD && millis() - offTime > DELAY_OFF) {
    on = false;

#if DEBUG
    Serial.print("Turning off...");
  } else if (on && distance > THRESHOLD) {
    Serial.print("Turning off, ");
    Serial.print(DELAY_OFF - (millis() - offTime));
    Serial.print("ms remaining");
#endif

  }

#if !SENSOR2
  digitalWrite(PIN_RELAY, on ? RELAY_ON : !RELAY_ON);
#endif

#if DEBUG
  Serial.println();
#endif

  ////////////////////////////////////////////////////////////////////////////////

// If sensor 2 is enabled, repeat all the stuff above, for sensor 2
#if SENSOR2

  wdt_reset();
  delay(DELAY_REFRESH);

  unsigned int distance2 = sonar2.convert_in(sonar2.ping_median(NUM_READINGS));

#if DEBUG
  Serial.print("Ping2: ");
  Serial.print(distance2);
  Serial.print("in -> ");
#endif

  if (distance2 == 0) {

#if DEBUG
    Serial.print("Ignoring...");
#endif

  } else if ((!on2 && distance2 > THRESHOLD) || (on2 && distance2 < THRESHOLD)) {
    onTime2 = offTime2 = millis();

#if DEBUG
    Serial.print("Stable: ");
    Serial.print(on2 ? "On" : "Off");
#endif

  } else if (!on2 && distance2 < THRESHOLD && millis() - onTime2 > DELAY_ON) {
    on2 = true;

#if DEBUG
    Serial.print("Turning on...");
  } else if (!on2 && distance2 < THRESHOLD) {
    Serial.print("Turning on, ");
    Serial.print(DELAY_ON - (millis() - onTime2));
    Serial.print("ms remaining");
#endif

  } else if (on2 && distance2 > THRESHOLD && millis() - offTime2 > DELAY_OFF) {
    on2 = false;

#if DEBUG
    Serial.print("Turning off...");
  } else if (on2 && distance2 > THRESHOLD) {
    Serial.print("Turning off, ");
    Serial.print(DELAY_OFF - (millis() - offTime2));
    Serial.print("ms remaining");
#endif

  }

  digitalWrite(PIN_RELAY, on || on2 ? RELAY_ON : !RELAY_ON);

#if DEBUG
  Serial.println();
  Serial.println("----------------------------------------");
#endif

#endif

}
