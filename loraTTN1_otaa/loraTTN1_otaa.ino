#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <BME280I2C.h>
#include <Adafruit_SI1145.h>
#include <CayenneLPP.h>

// meteofrance.fr : "En moyenne, la pression atmosphérique diminue de 1 hPa tous les 8 mètres"
#define ALT 209
#define COR (ALT/8.0)

// Taille maximum de message
#define MAX_SIZE 100

// Notre message au format LPP
CayenneLPP lpp(MAX_SIZE);

// On déclare les capteurs
Adafruit_SI1145 uv = Adafruit_SI1145();
BME280I2C bme;

// Identifiant tâche
static osjob_t sendjob;

// Emission toutes les 5 mn
const unsigned tx_interval = 300;

// Brochage
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

// Identifiant d'application
// Format little-endian/LSB
static const u1_t PROGMEM appeui[8] = { 0x07, 0x51, 0x00, 0xF0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, appeui, 8);}

// Identifiant de node/périphérique
// Format little-endian/LSB
static const u1_t PROGMEM deveui[8] = { 0x99, 0x94, 0x74, 0x13, 0x68, 0x8E, 0x09, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, deveui, 8);}

// Clé de l'application
// Format big-endian/MSB
static const u1_t PROGMEM appkey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, appkey, 16);}

// Gestion des événements
void onEvent(ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT")); break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND")); break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED")); break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED")); break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING")); break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            // link check validation non supporté avec TTN
            LMIC_setLinkCheckMode(0);
            LMIC_setDrTxpow(DR_SF12,14);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1")); break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED")); break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED")); break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (avec attente RX)"));
            if(LMIC.dataLen) {
                // données reçues ?
                Serial.print(F("Data RX: "));
                Serial.write(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
                Serial.println();
            }
            // Planifier la prochaine émission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(tx_interval), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC")); break;
        case EV_RESET:
            Serial.println(F("EV_RESET")); break;
        case EV_RXCOMPLETE:
            Serial.println(F("EV_RXCOMPLETE")); break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD")); break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE")); break;
         default:
            Serial.println(F("inconnu")); break;
    }
}

// Notre tâche
void do_send(osjob_t* j){
  // variable pour les capteurs
  float temp, hum, pres, uvindex, lum;

  // Communication déjà en cour ?
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, pas d'envoi"));
  } else {
    // lecture des capteurs
    bme.read(pres, temp, hum, true, B001);
    uvindex = uv.readUV()/100.0;
    lum = uv.readVisible();

    // Affichage des valeurs
    Serial.print("uv: ");
    Serial.print(uvindex);
    Serial.print("\tlum: ");
    Serial.print(lum);
    Serial.print("\ttemp: ");
    Serial.print(temp);
    Serial.print("\thum: ");
    Serial.print(hum);
    Serial.print("\tpress: ");
    Serial.println(pres+COR);

    // Composition du message
    lpp.reset();
    lpp.addTemperature(0, temp);
    lpp.addRelativeHumidity(1, hum);
    lpp.addBarometricPressure(2, pres+COR);
    lpp.addLuminosity(3,(int)uvindex);
    lpp.addLuminosity(4,(int)lum);

    // Enregistrement du message à envoyer
    LMIC_setTxData2(1, lpp.getBuffer(), lpp.getSize(), 0);
    Serial.println(F("Packet queued"));
  }
}

// configuration
void setup() {
  Serial.begin(115200);
  Serial.println(F("Go Go Go !"));

  // initialisation capteurs
  while (!bme.begin()) {
    Serial.println("Erreur BME280! !");
    delay(2000);
  }
  while (!uv.begin()) {
    Serial.println("Erreur SI1145 !");
    delay(2000);
  }

  // LMIC init
  os_init();
  // Reset du module/shield
  LMIC_reset();

  // Démarrage tâche
  do_send(&sendjob);
}

void loop() {
  // boucle de gestion
  os_runloop_once();
}

