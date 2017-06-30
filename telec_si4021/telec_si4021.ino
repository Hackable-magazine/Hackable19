#include <SPI.h>
#include <RCSwitch.h>
#include "si4021.h"
#include "telec.h"

/* Memo SPI Uno/Nano 
11 MOSI
12 MISO
13 SCK
10 SEL/CS
 8 FSK
*/

#define CS 10
#define FSK 8
#define BOUT 4

RCSwitch mySwitch = RCSwitch();

struct cfg {
  float freq;
  unsigned int attenuation;
} maconfig = { 433.92, PS_0DB };


/***** Fonctions pour calculer les bits de configuration *****/

// réglage fréquence 
uint16_t freqset(float freq) {
  int c1;
  int c2;
  uint16_t ret;
  if(freq < 440) {
    c1 = FS_BAND433_C1;
    c2 = FS_BAND433_C2;
  } else if (freq < 880) {
    c1 = FS_BAND868_C1;
    c2 = FS_BAND868_C2;
  } else if (freq < 930) {
    c1 = FS_BAND915_C1;
    c2 = FS_BAND915_C2;
  } else {
    return(0);
  }
  ret = (((freq/10/c1)-c2)*4000)+0.5; // rounding
  if(ret > 3903 || ret < 96)
    return(0);
  return(ret | FREQSET);
}

// réglage configuration, capa et bande
uint16_t confset(float freq) {
  if(freq < 440) {
    return(CONFSET | CS_CLK10000 | CS_BAND433 | CS_CAPA160);
  } else if (freq < 880) {
    return(CONFSET | CS_CLK10000 | CS_BAND868 | CS_CAPA160);
  } else if (freq < 930) {
    return(CONFSET | CS_CLK10000 | CS_BAND915 | CS_CAPA160);
  } else {
    return(0);
  }
}

// réglage modulation et puissance
uint8_t powerset(unsigned int attenuation) {
  if(attenuation < 8)
  return(POWERSET | PS_OOK | attenuation);
  return(0);
}

// réglage ampli
uint16_t powerman() {
  return(POWERMAN | PM_OSC | PM_SYNT | PM_AMP);
}

/***** Fonctions utilitaires  *****/

// affichage binaire (debug)
void printbit(uint8_t val) {
  for(int i=sizeof(val)*8-1; i >= 0; i--) {
    if(val & (1 << i))
      Serial.print("1");
    else
      Serial.print("0");
  }
  Serial.println("");
}

// Envoi de valeur au si4021 en SPI
// (ce n'est pas une erreur, en C++ ceci est la surchare de fonction)
void sendToSi4021(uint8_t val) {
  digitalWrite(CS, LOW);
  SPI.transfer(val);
  digitalWrite(CS, HIGH);
}
void sendToSi4021(uint16_t val) {
  digitalWrite(CS, LOW);
  SPI.transfer(val >> 8);
  SPI.transfer(val & 255);
  digitalWrite(CS, HIGH);
}

/***** Croquis *****/

// configuration
void setup() {
  uint16_t cbits;
  uint16_t fbits;
  uint16_t pwmanbits;
  uint8_t  pwbits;

  // configuration ports en sortie
  pinMode(CS, OUTPUT);
  pinMode(FSK, OUTPUT);
  pinMode(BOUT, INPUT);

  Serial.begin(115200);

  // CS à l'état haut
  // le module n'est PAS "sélectionné" et
  // ne traite donc PAS les éventuelles données SPI
  digitalWrite(CS, HIGH);
  
  // Configuration SPI
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  // On utilise la sortie FSK (broche 8)
  mySwitch.enableTransmit(FSK);

  // Option : durée de l'impulsion
  // mySwitch.setPulseLength(125);
  // Option : protocole (1 fonctionne la plupart du temps)
  mySwitch.setProtocol(1);
  // Option : nombre de répétition du message
  mySwitch.setRepeatTransmit(5);
  
  delay(100);

  // Configuration :
  Serial.println("Configuration Setting");
  if(!(cbits = confset(maconfig.freq))) {
      Serial.println("ERROR : bad config!");
      while(1);
  } else {
    sendToSi4021(cbits);
  }

  // configuration de la fréquence sur 433.92 MHz
  Serial.println("Frequency Setting");
  if(!(fbits = freqset(maconfig.freq))) {
    Serial.println("ERROR : bad frequency!");
  } else {
    sendToSi4021(fbits);
  }

  // Configuration de l'emission
  Serial.println("Power Setting");
  if(!(pwbits = powerset(maconfig.attenuation))) {
    Serial.println("ERROR : bad power settings!");
  } else {
    sendToSi4021(pwbits);
  }

  // Configuration de l'alimentation
  Serial.println("Power Management");
  if(!(pwmanbits = powerman())) {
    Serial.println("ERROR : bad power management settings!");
  } else {
    sendToSi4021(pwmanbits);
  }

  delay(1000);

/*
  Serial.println("ON");
  mySwitch.send(C1B1_ON);
  delay(50);
  mySwitch.send(C1B2_ON);
  delay(50);
  mySwitch.send(C1B4_OFF);
  delay(3000);

  Serial.println("OFF");
  mySwitch.send(C1B4_ON);
  delay(50);
  mySwitch.send(C1B1_OFF);
  delay(50);
  mySwitch.send(C1B2_OFF);
*/
}

// boucle principale
void loop() {
  if(!digitalRead(BOUT)) {
    mySwitch.send("010101010101000000000011");
    delay(500);
  }
}
