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

#define VBAT_PIN PA1


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

double battery_calc_charge(double voltage) {    
    //calibration table meant for a 12s battery
    int capacities[] = {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 0};
    double voltages[] = {4.2, 4.15, 4.11, 4.08, 4.02, 3.98, 3.95, 3.91, 3.87, 3.85, 3.84, 3.82, 3.8, 3.79, 3.77, 3.75, 3.73, 3.71, 3.69, 3.61, 3.27};

    // Clip voltage within the range
    if (voltage > voltages[0]) {
        voltage = voltages[0];
    } else if (voltage < voltages[20]) {
        voltage = voltages[20];
    }

    // Linear interpolation
    int i;
    for (i = 0; i < 20; ++i) {
        if (voltage >= voltages[i]) {
            break;
        }
    }

    // Calculate charge percentage using linear interpolation
    double charge = capacities[i] + (capacities[i + 1] - capacities[i]) * (voltage - voltages[i]) / (voltages[i + 1] - voltages[i]);

    return charge;
}

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
    pinMode(B2,INPUT_PULLUP);
    pinMode(B3,INPUT_PULLUP);
    analogReadResolution(12);
    digitalWrite(ld4,0);
    Serial.begin(9600);
    Serial.println("\nLORA TESTS");
    LoRa_init();
}

void loop()
{
    if(!digitalRead(B3)){
        float VDDA = 1.2 * 4095 / analogRead(AVREF);
        float VBAT = 2* VDDA * analogRead(VBAT_PIN)/4095;  

        double charge = battery_calc_charge(VBAT);
        Serial.print("VDDA : ");
        Serial.println(VDDA);
        Serial.print("VBAT : ");
        Serial.println(VBAT);
        digitalWrite(ld1,charge>75);
        digitalWrite(ld2,charge>50);
        digitalWrite(ld3,charge>25);
        digitalWrite(ld4,1);
    }
    else{
        digitalWrite(ld1,0);
        digitalWrite(ld2,0);
        digitalWrite(ld3,0);
    }
    
    if(!digitalRead(B2)){
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