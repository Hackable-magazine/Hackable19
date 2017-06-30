#include <LoRaWan.h>
#include <CayenneLPP.h>

// Taille maximum de message
#define MAX_SIZE 100

// Notre message au format LPP
CayenneLPP lpp(MAX_SIZE);

char buffer[256];

void setup(){
    SerialUSB.begin(115200);
    while(!SerialUSB);

    SerialUSB.println("Go go go!");
    
    lora.init();

    SerialUSB.print("Battery: ");
    SerialUSB.print(lora.getBatteryVoltage());
    SerialUSB.println("V");

    memset(buffer, 0, 256);
    lora.getVersion(buffer, 256, 1);
    SerialUSB.print(buffer); 
    
    memset(buffer, 0, 256);
    lora.getId(buffer, 256, 1);
    SerialUSB.print(buffer);

    // void setKey(char *NwkSKey, char *AppSKey, char *AppKey);
    lora.setKey(NULL, NULL, "00000000000000000000000000000000"); // votre AppKey
    lora.setDeciveMode(LWOTAA);
    lora.setDataRate(DR0, EU868);
    lora.setAdaptiveDataRate(true);
    
    lora.setChannel(0, 868.1);
    lora.setChannel(1, 868.3);
    lora.setChannel(2, 868.5);
    lora.setChannel(3, 867.1);
    lora.setChannel(4, 867.3);
    lora.setChannel(5, 867.5);
    lora.setChannel(6, 867.7);
    
    lora.setReceiceWindowFirst(1);
    lora.setReceiceWindowSecond(869.5, DR3);
    
    lora.setPower(20);

    SerialUSB.println(">>> SETUP DONE"); 
    
    if(!lora.setOTAAJoin(JOIN,10)) {
      SerialUSB.println(">>> JOIN ERROR");
      while(1) { ;; }
    }

    SerialUSB.println(">>> JOIN DONE"); 
    
}

void loop(void) {
    SerialUSB.println(">>> LOOP loop"); 
    bool result = false;
    
    lpp.reset();
    lpp.addTemperature(0, 42); // valeur arbitraire pour test
    result = lora.transferPacket(lpp.getBuffer(), lpp.getSize());
    
    if(result){
        short length;
        short rssi;
        
        memset(buffer, 0, 256);
        length = lora.receivePacket(buffer, 256, &rssi);
        
        if(length){
            SerialUSB.print("Length is: ");
            SerialUSB.println(length);
            SerialUSB.print("RSSI is: ");
            SerialUSB.println(rssi);
            SerialUSB.print("Data is: ");
            for(unsigned char i = 0; i < length; i ++){
                SerialUSB.print("0x");
                SerialUSB.print(buffer[i], HEX);
                SerialUSB.print(" ");
            }
            SerialUSB.println();
        }
    }

    delay(60000);
}


