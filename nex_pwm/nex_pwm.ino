#include <SoftwareSerial.h>
#include <Nextion.h>

// port série sur broche 4 et 5
SoftwareSerial nexSerial(4, 5); // RX, TX

// composants de l'interface
NexNumber num0        = NexNumber(0, 1, "num0");
NexProgressBar barre0 = NexProgressBar(0, 3, "barre0");
NexButton b0          = NexButton(0, 3, "b0");
NexButton b1          = NexButton(0, 4, "b1");
NexSlider h0          = NexSlider(0, 5, "h0");

// liste d'événements/composants à écouter
NexTouch *nex_listen_list[] = 
{
    &b0,
    &b1,
    &h0,
    NULL
};

// fonction lancée lors du relâchement de b0
void b0Callback(void *ptr) {
  uint32_t val;
  num0.getValue(&val);
  if (val >= 5)
    val -= 5;
  else
    val = 0;
  barre0.setValue(map(val,0,255,0,100));
  num0.setValue(val);
  h0.setValue(val);
  analogWrite(6, val);
}

// fonction lancée lors du relâchement de b1
void b1Callback(void *ptr) {
  uint32_t val;
  num0.getValue(&val);
  if (val <=250)
    val += 5;
  else
    val = 255;
  barre0.setValue(map(val,0,255,0,100));
  num0.setValue(val);
  h0.setValue(val);
  analogWrite(6, val);
}

// fonction lancée lors du relâchement de h1
void h0Callback(void *ptr) {
  uint32_t val;
  h0.getValue(&val);
  barre0.setValue(map(val,0,255,0,100));
  num0.setValue(val);
  analogWrite(6, val);
}

// configuration
void setup(void) {
  pinMode(6, OUTPUT);
  Serial.begin(115200);
  // cette fonction retourne vrai si l'écran répond
  if(nexInit()) {
    Serial.println("setup ok");
  } else {
    Serial.println("setup error !!!");
    while(1){;}
  }
  // configuration des fonctions à appeler
  b0.attachPop(b0Callback);
  b1.attachPop(b1Callback);
  h0.attachPop(h0Callback);
}

// boucle principale
void loop(void) {
  // attente d'événements
  nexLoop(nex_listen_list);
}

