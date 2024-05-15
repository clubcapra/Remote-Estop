/*
    LoRa: Send and Receive between two boards
    More info: 
        https://github.com/sandeepmistry/arduino-LoRa/blob/master/API.md
*/

#include <Arduino.h>
#include <LoRa.h>
#include <Timer.h>

//#define RCV

int rcv_flg=0;
int state = 0;

#define REFRESH_PERIOD_MS 50

#define TIMEOUT 500

void onReceive(int packetSize) // ! from isr
{
    Serial.print("RECIEVED : ");
    state = LoRa.read();
    Serial.println(state);
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
    //LoRa.onReceive(onReceive);
    LoRa.receive();
    Serial.printf("LoRa Init\n");
}

void LoRa_send()
{
    LoRa.beginPacket();
    #ifdef RCV
    LoRa.print("RX GOOD");
    #else
    LoRa.write(1);
    #endif
    LoRa.endPacket();
    Serial.println("Packet sent");
    LoRa.receive();
}

void send_at_interval();

#define ld1 PB5
#define ld2 PB4
#define ld3 PB3
#define ld4 PA15



void setup()
{
    pinMode(ld1,OUTPUT);
    digitalWrite(ld1,1);
    pinMode(ld2,OUTPUT);
    digitalWrite(ld2,1);
    pinMode(ld3,OUTPUT);
    digitalWrite(ld3,1);
    pinMode(ld4,OUTPUT);
    digitalWrite(ld4,1);
    delay(1000);
    Serial.begin(9600);
    Serial.println("\nLORA TESTS");
    LoRa_init();
}

void loop()
{
    static unsigned long previousRCV = 0;
    #ifdef RCV
    if(rcv_flg){
        rcv_flg = 0;
        LoRa_send();
        digitalWrite(PB8,state);
        previousRCV = millis();
    }
    else if(millis() - previousRCV > TIMEOUT){
        state = 0;
        digitalWrite(PB8,state);
    }
    #else
    send_at_interval(); // Call the function to check and execute the action
    #endif
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