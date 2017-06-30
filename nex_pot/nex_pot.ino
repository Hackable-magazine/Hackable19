#include <SoftwareSerial.h>
#include <Nextion.h>

// port série sur broche 4 et 5
SoftwareSerial nexSerial(4, 5); // RX, TX

// composants de l'interface
NexNumber num0        = NexNumber(0, 1, "num0");
NexGauge jauge0       = NexGauge(0, 2, "jauge0");
NexProgressBar barre0 = NexProgressBar(0, 3, "barre0");

// configuration
void setup(void) {
  Serial.begin(115200);
  // cette fonction retourne vrai si l'écran répond
  if(nexInit()) {
    Serial.println("setup ok");
  } else {
    Serial.println("setup error !!!");
    while(1){;}
  }
}

// valeur précédente
int pval = 0;

// boucle principale
void loop(void) {
  // lecture analogique et transposition de valeurs
  int val = map(analogRead(A0),0,1023,0,180);
  // on ne change l'interface sur si la valeur change
  if(val != pval) {
    // changement de la barre de progression
    barre0.setValue(map(val,0,180,0,100));
    // changement de la jauge
    jauge0.setValue(val);
    // changement du nombre affiché
    num0.setValue(val);
    // mise à jour
    pval=val;
  }
  // pause
  delay(20);
}

