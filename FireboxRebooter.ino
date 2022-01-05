/*
 * ATtinyCore - ATtiny 25/45/85 (No bootloader)
 * 
 * 
 */

// Pin definitions
#define PWRBTN   4
#define PWRSENSE 0
#define LEDSENSE 3
#define WAITLED  1
#define DBGPWM   2

// Timer Settings
#define RSTTIME  5000  // time in ms until a shutdown is forced when the SFP LED stays on
#define BOOTTIME 10000 // time in ms to wait after power on before going to sense mode again
#define OFFTIME  2000  // time in ms until reboot after shutdown
#define MAXHOLD  10000 // maximum time in ms that the power button is held

void setup() {
  pinMode(PWRBTN, OUTPUT);
  digitalWrite(PWRBTN, LOW);
  pinMode(WAITLED, OUTPUT);
  digitalWrite(WAITLED, LOW);
  pinMode(PWRSENSE, INPUT);
  pinMode(LEDSENSE, INPUT);

  // wait for power being turned on before doing anything at all
  bootWait();
}

bool ledstate;
unsigned long int ledtime; 
unsigned long int seconds;

void loop() {
  // check if power is on, if not go to waiting state
  if (digitalRead(PWRSENSE) == LOW) bootWait();

  // check sense LED state
  bool ledsense = digitalRead(LEDSENSE);
  if (ledsense != ledstate) {
    ledstate = ledsense;
    if (ledsense == HIGH) ledtime = millis(); // store the time of LED turning on
  }
  if (ledstate == HIGH && ledtime + RSTTIME <= millis()) FBReset(); // start reset cycle when LED is on for more than RSTTIME ms
  
  // flash wait LED once every other second to show standby
  if (seconds + 2000 <= millis()) {
    seconds = millis();
    digitalWrite(WAITLED, HIGH);
    delay(5);
    digitalWrite(WAITLED, LOW);
  }
}

void bootWait() {
  // Wait for power being turned on
  while (digitalRead(PWRSENSE) == LOW) { 
    //digitalWrite(WAITLED, !digitalRead(WAITLED)); // toggle pin to flash LED
    PINB = 1 << WAITLED; // toggle pin to flash LED
    delay(500); 
  }

  // wait for POST being finished before starting the loop
  digitalWrite(WAITLED, HIGH);
  
  // make sure the box isnt switched off while waiting for the sense led to go off
  unsigned long int timer = millis();
  while (digitalRead(PWRSENSE) == HIGH && (timer + BOOTTIME > millis() || digitalRead(LEDSENSE) == HIGH)); //do nothing
  
  digitalWrite(WAITLED, LOW);
  
  // if power is off at this point, start bootWait again (This only happens when box was switched off while waiting)
  if (digitalRead(PWRSENSE) == LOW) bootWait();
}

void FBReset() {  
  // Switch Firebox off 
  switchPower(LOW); 
  
  // pause 'OFFTIME' ms
  digitalWrite(WAITLED, HIGH);
  delay(OFFTIME);
  digitalWrite(WAITLED, LOW);
  
  // Switch Firebox on again and wait BOOTTIME ms
  switchPower(HIGH); 

  bootWait();
}

void switchPower(bool power) {
  // press and hold power button
  digitalWrite(PWRBTN, HIGH); 
  
  // wait until either power is switched or MAXHOLD time is up
  unsigned long int timestamp = millis();
  while (digitalRead(PWRSENSE) != power && timestamp + MAXHOLD > millis()) {
    PINB = 1 << WAITLED; // toggle pin
    delay(200); 
  }

  // release power button
  digitalWrite(PWRBTN, LOW);


}
