// Pro Micro reads the LIN data, and monitors LIN buttons and dash buttons. It is located in the dash. 
// It sends the commands over a serial connection to another controller. 
// The second controller is a Digispark and is in the battery box of the car and actuates the relays.
// Send 1, 2, 3, 4 to turn those relays on, Send, 5, 6, 7, 8 to turn them off.

#include <SoftSerial.h>
#include <TinyPinChange.h>

#define RELAY1_PIN 0
#define RELAY2_PIN 1
#define RELAY3_PIN 3
#define RELAY4_PIN 4

#define RX_PIN 2
#define TX_PIN 5

SoftSerial mySerial(RX_PIN, TX_PIN);

void setup() 
{
  mySerial.begin(9600);
  
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);

}

void loop() {

  if(mySerial.available()) {
    switch(mySerial.read()) {
      case 1:
        toggleRelay(RELAY1_PIN,0);
        break;
      case 2:
        toggleRelay(RELAY2_PIN,0);
       break;
       case 3:
        toggleRelay(RELAY3_PIN,0);
       break;
       case 4:
        toggleRelay(RELAY4_PIN,0);
       break;
       case 5:
        toggleRelay(RELAY1_PIN,1);
       break;
       case 6:
        toggleRelay(RELAY2_PIN,1);
       break;
       case 7:
        toggleRelay(RELAY3_PIN,1);
       break;
       case 8:
        toggleRelay(RELAY4_PIN,1);
       break;
    }
  }
}

void toggleRelay(uint8_t relay, uint8_t state) { // digitalWrite must recieve HIGH or LOW, not 1 or 0. 
  digitalWrite(relay, state ? HIGH : LOW); 
}
