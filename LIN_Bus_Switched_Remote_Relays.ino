// Pro Micro reads the LIN data, and monitors LIN buttons and dash buttons. It is located in the dash. 
// It sends the commands over a serial connection to another controller. 
// The second controller is a Digispark and is in the battery box of the car and actuates the relays.

#include <avr/sleep.h>
#include <SoftwareSerial.h>
#include <LIN_BUS_lib-Skuzee.h>

// define pins for LIN transceiver communication
#define CS_PIN 2
#define LIN_TX_PIN 1
#define LIN_RX_PIN 0

// define pins for dash switches
#define SW_DASH1 3
#define SW_DASH2 4
#define SW_DASH3 5
#define SW_FOOT 6

// define which parts of the data frame do what. Sync, PID, 2-8 Data bytes, Checksum, Breakfield.
#define LIN_BAUD 9600UL
#define SYNC 0
#define PID 1
#define DATA 2
#define CHECKSUM 10
#define BREAKFIELD 11

// helpful for for looping through switch and relay states.
typedef union {
  uint8_t rawData[4];
  struct {
    uint8_t sw_dash1;
    uint8_t sw_dash2;
    uint8_t sw_dash3;
    uint8_t sw_foot;
  };
} tSwitches;

typedef union {
  uint8_t rawData[4];
  struct {
    uint8_t RELAY1_STATE;
    uint8_t RELAY2_STATE;
    uint8_t RELAY3_STATE;
    uint8_t RELAY4_STATE;
  };
} tRelays;

// car high/low beam switch status
typedef union {
  uint8_t rawData[4];
  struct {
    uint8_t LIN_BEAM_HALF;
    uint8_t LIN_BEAM_FULL;
    uint8_t LIN_SW3;
    uint8_t LIN_SW4;
  };
} tLINbuttons;

// raw LIN data.
typedef union {
  uint8_t rawData[11];
  struct {
    uint8_t sync;
    uint8_t pid;
    uint8_t data[8];
    uint8_t checksum;
    uint8_t dlc; //data length in bytes
    uint8_t id;
  };
} tLIN;

LINtransceiver myLIN(LIN_TX_PIN, LIN_RX_PIN, 9600, CS_PIN);
tSwitches switches;
tRelays relays;
tLINbuttons LINbuttons;

SoftwareSerial SerialDigi(14, 15); // RX, TX


void setup() {

  Serial.begin(115200); // Serial is USB serial!
  // Serial1 is the Tx/Rx pins that make a serial connection to the LIN transciever.
  SerialDigi.begin(9600); // SerialDigi is the software serial conenction to the digispark that controls the relays.
  


  for (int i = SW_DASH1; i <= SW_FOOT; i++) // set switch pins to inputs.
    pinMode(i, INPUT_PULLUP);

  for (int i = 0; i <= 4; i++) { // initiate all relays, switches, and LIN buttons as off.
  relays.rawData[i]=i+1;
  switches.rawData[i]=0;
  LINbuttons.rawData[i]=0;
  }
}


void loop() {

//    updateSwitches();
//    updateRelays();
//    sendRelayData();
   if(millis()- myLIN.getTimestampOfLastByte() >=30000) // if it has been 10 seconds since the last LIN data signal...
     arduinoToSleep(); // Put the arduino to sleep to save power.

  if (myLIN.read()) { // returns TRUE when new LIN data is ready
    updateLINbuttons(myLIN.getData());
  }
 

}

void printData(tLIN LINdata) {
  for (int i = 0; i < 11; i++) {
    switch (i) {
      case SYNC:
        Serial.println("");
        Serial.print("Sync:");
        Serial.print(LINdata.sync, HEX);
        Serial.print(",");
        break;
      case PID:
        Serial.print("PID:");
        Serial.print(LINdata.pid, HEX);
        Serial.print(",ID:");
        Serial.print(LINdata.id, HEX);
        Serial.print(",Data:");
        break;
      case DATA:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        Serial.print(LINdata.rawData[i], HEX);
        Serial.print(",");
        break;
      case CHECKSUM:
        Serial.print("CHECKSUM:");
        Serial.print(LINdata.rawData[i], HEX);
        break;
      default:
        Serial.println("ERROR");
    }
  }
}

void arduinoToSleep() { // puts arduino to sleep if there is no communication.
  Serial.println("");
  Serial.println("Good Night!");
  delay(1000);
  Serial.flush();
  Serial.end();
  Serial1.flush();
  Serial1.end();
  sleep_enable();
  pinMode(LIN_RX_PIN,INPUT);
  attachInterrupt(digitalPinToInterrupt(LIN_RX_PIN), arduinoWakeUp, FALLING);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sei();
  sleep_cpu();
  Serial.println("Good Morning!");
}

void arduinoWakeUp() {
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(LIN_RX_PIN));
  Serial1.begin(LIN_BAUD,SERIAL_8N1);
  Serial.println("Good Morning!");
}

void updateSwitches() { // reads dash switch state and update.
  for (int i = 0; i < 4; i++) {
    switches.rawData[i] = digitalRead(i + SW_DASH1); // sw_dash1 = rawData[0] = PIN 3
  }
}

void updateLINbuttons(LIN_Data_t LIN_Data) { // updates lin switch state
  if (LIN_Data.id == 22) {
    LINbuttons.LIN_BEAM_HALF = (LIN_Data.data[0] & 0b01000000 == 0b01000000);
    LINbuttons.LIN_BEAM_FULL = (LIN_Data.data[0] & 0b10000000 == 0b10000000);
  }
}

void updateRelays() { 
  // Trying to avoid any issue with communication, specific values for each relay on and off
  // 1/5 RELAY1 OFF/ON.
  // 2/6 RELAY2 OFF/ON. 
  // 3/7 RELAY3 OFF/ON. 
  // 4/8 RELAY4 OFF/ON.

  //Case for RELAY1
  relays.RELAY1_STATE = 1 + 4*(switches.sw_dash1 && switches.sw_foot || switches.sw_dash2 && LINbuttons.LIN_BEAM_HALF); //1 OFF 5 ON
}

void sendRelayData() { // To prevent and data loss from affect remote relays we use unique numbers for each relay off/on state
  for(int i=0;i<4;i++) {
    SerialDigi.write(relays.rawData[i]);
  }

}
