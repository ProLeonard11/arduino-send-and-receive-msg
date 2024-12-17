/*
  Communications v5

  Transmits data using a white LED and recieves it using a photoresistor

*/


// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const int trigPin = 9;
const int echoPin = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define STX 0x70
#define ETX 0x71

char txButton, txTilt, txPot, txUltraSound, txB, txC, txD;
char txBuffer[8] = { 0, 0, 0, 0, 0, 0, 0, ETX };
char rxBuffer[7];
char rxButton, rxTilt, rxPot, rxUltraSound, rxB, rxC, rxD;
int rx_index;

int buttonPin = 8;
int buzzer = 7;  //the pin of the active buzzer


void readInputs() {
  // Reads the inputs in the mini-projects
  // Uses txButton, txTilt, txPot, txA, txB, txC, txD;
  if (digitalRead(buttonPin) == LOW) {
    txButton = 1;
  } else {
    txButton = 0;
  }

  // Read the input of the ultra sound sensor

  long duration, cm;   // These variables will be needed later when we convert time(duration) into centimetres

  
  digitalWrite(trigPin, LOW); // This will output the ultra sounds high pitch frequency
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);


  // convert the time into a distance
  cm = microsecondsToCentimeters(duration);

  txUltraSound = cm;
}

void writeOutputs() {
  // Writes the outputs in the mini-projects
  // Uses rxButton, rxTilt, rxPot, rxA, rxB, rxC, rxD;
  digitalWrite(buzzer, rxButton);
  
  lcd.clear();
  lcd.print("Distance: ");
  lcd.print((int)rxUltraSound);
  lcd.print("cm");

   
}



int ledState = LOW;  // ledState used to set the LED


char encrypt(char in_char) {
  char out_char;

  out_char = in_char;

  return out_char;
}


char decrypt(char in_char) {
  char out_char;

  out_char = in_char;

  return out_char;
}



// the setup routine runs once when you press reset:
void setup() {
  // set the digital pin as output:
  pinMode(6, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);  //initialize the buzzer pin as an output

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}


const long txInterval = 200;  // interval at which to tx bit (milliseconds)
int tx_state = 0;
char tx_char = 'H';
char chr;
unsigned long previousTxMillis = 0;  // will store last time LED was updated

char tx_string[] = "Hello World";
#define TX_START_OF_TEXT -1
int tx_string_state = TX_START_OF_TEXT;



char getTxChar() {
  char chr;

  switch (tx_string_state) {
    case TX_START_OF_TEXT:
      tx_string_state = 0;
      txBuffer[0] = txButton;
      txBuffer[1] = txTilt;
      txBuffer[2] = txPot;
      txBuffer[3] = txUltraSound;
      txBuffer[4] = txB;
      txBuffer[5] = txC;
      txBuffer[6] = txD;
      return STX;
      break;

    default:
      chr = txBuffer[tx_string_state];
      tx_string_state++;
      if (chr == ETX) /* End of string? */
      {
        tx_string_state = TX_START_OF_TEXT; /* Update the tx string state to start sending the string again */
        return ETX;                         /* Send End of Text character */
      } else {
        return chr; /* Send a character in the string */
      }
      break;
  }
}

void txChar() {
  unsigned long currentTxMillis = millis();

  if (currentTxMillis - previousTxMillis >= txInterval) {
    // save the last time you blinked the LED (improved)
    previousTxMillis = previousTxMillis + txInterval;  // this version catches up with itself if a delay was introduced

    switch (tx_state) {
      case 0:
        chr = encrypt(getTxChar());
        digitalWrite(6, HIGH); /* Transmit Start bit */
        tx_state++;
        break;

      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
        if ((chr & 0x40) != 0) /* Transmit each bit in turn */
        {
          digitalWrite(6, HIGH);
        } else {
          digitalWrite(6, LOW);
        }
        chr = chr << 1; /* Shift left to present the next bit */
        tx_state++;
        break;

      case 8:
        digitalWrite(6, HIGH); /* Transmit Stop bit */
        tx_state++;
        break;

      default:
        digitalWrite(6, LOW);
        tx_state++;
        if (tx_state > 20) tx_state = 0; /* Start resending the character */
        break;
    }
  }
}



const long rxInterval = 20;  // interval at which to read bit (milliseconds)
int rx_state = 0;
char rx_char;
unsigned long previousRxMillis = 0;  // will store last time LED was updated
int rx_bits[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


void rxChar() {
  unsigned long currentRxMillis = millis();
  int sensorValue;
  int i;

  if (currentRxMillis - previousRxMillis >= rxInterval) {
    // save the last time you read the analogue input (improved)
    previousRxMillis = previousRxMillis + rxInterval;  // this version catches up with itself if a delay was introduced

    sensorValue = analogRead(A0);
    //Serial.println(rx_state);

    switch (rx_state) {
      case 0:
        if (sensorValue >= 40) {
          rx_bits[0]++;
          rx_state++;
        }
        break;

      case 100:
        if ((rx_bits[0] >= 6) && (rx_bits[8] >= 6)) /* Valid start and stop bits */
        {
          rx_char = 0;

          for (i = 1; i < 8; i++) {
            rx_char = rx_char << 1;
            if (rx_bits[i] >= 6) rx_char = rx_char | 0x01;
          }
          rx_char = decrypt(rx_char);

          Serial.println(rx_char + 0);

          switch (rx_char) {
            case STX:
              rx_index = 0;
              break;

            case ETX:
              rxButton = rxBuffer[0];
              rxTilt = rxBuffer[1];
              rxPot = rxBuffer[2];
              rxUltraSound = rxBuffer[3];
              rxB = rxBuffer[4];
              rxC = rxBuffer[5];
              rxD = rxBuffer[6];
              rx_index = 0;
              break;

            default:
              rxBuffer[rx_index] = rx_char;
              rx_index++;
              break;
          }
        } else {
          Serial.println("Rx error");
        }
        //        for (i = 0; i < 10; i++)  /* Print the recieved bit on the monitor - debug purposes */
        //        {
        //          Serial.println(rx_bits[i]);
        //        }
        for (i = 0; i < 10; i++) {
          rx_bits[i] = 0;
        }
        rx_state = 0;
        break;

      default:
        if (sensorValue >= 40) {
          rx_bits[rx_state / 10]++;
        }
        rx_state++;
        break;
    }
  }
}



// the loop routine runs over and over again forever:
void loop() {
  readInputs();
  txChar();
  rxChar();
  writeOutputs();
}


long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}