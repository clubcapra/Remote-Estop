#include <Arduino.h>
#include <LoRa.h>
#include <Timer.h>

int rcv_flg=0;
int state = 0;

#define REFRESH_PERIOD_MS 350

#define TIMEOUT 500

#define ld1 PB5
#define ld2 PB4
#define ld3 PB3
#define ld4 PA15

#define B1 PA8
#define B2 PA9
#define B3 PA10


void onReceive(int packetSize) // ! from isr
{
    state = LoRa.read();
    while (LoRa.available()){
        LoRa.read();
    }
    rcv_flg=1;
}

void LoRa_init()
{
    SPI.setMISO(PA6);
    SPI.setMOSI(PA7);
    SPI.setSCLK(PA5);
    LoRa.setPins(PA4, PA2, PA3);
    LoRa.setTxPower(20);
    
    LoRa.setSyncWord(0xBE);
    if (!LoRa.begin(904600E3)) // frequency
    {
        Serial.println("\n[ERROR] LoRa init failed");
        abort();
    }
    LoRa.onReceive(onReceive);
    Serial.printf("LoRa Init\n");
}

void LoRa_send()
{
    LoRa.beginPacket();
    LoRa.write(!digitalRead(B1));
    LoRa.endPacket();
    Serial.println("Packet sent");
    LoRa.receive();
}

void send_at_interval();

void setup()
{
    pinMode(ld1,OUTPUT);
    digitalWrite(ld1,0);
    pinMode(ld2,OUTPUT);
    digitalWrite(ld2,0);
    pinMode(ld3,OUTPUT);
    digitalWrite(ld3,0);
    pinMode(ld4,OUTPUT);
    pinMode(B1,INPUT_PULLUP);
    digitalWrite(ld4,1);
    delay(1000);
    Serial.begin(9600);
    Serial.println("\nLORA TESTS");
    LoRa_init();
}

void loop()
{
    static unsigned long previousRCV = 0;
    send_at_interval(); // Call the function to check and execute the action
    if(rcv_flg){
        rcv_flg = 0;
        digitalWrite(ld4,state);
        previousRCV = millis();
    }
    else if(millis() - previousRCV > TIMEOUT){
        state = 0;
        digitalWrite(ld4,state);
    }
}

void send_at_interval() {
    static unsigned long previousMillis = 0;
    // Check if it's time to execute the action
    if (millis() - previousMillis >= REFRESH_PERIOD_MS) {
        // Save the last time the action was executed
        previousMillis = millis();
        // Call your function here
        LoRa_send();
    }
}