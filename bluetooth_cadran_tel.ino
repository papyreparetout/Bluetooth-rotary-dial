#include <Arduino.h>

/*
Rotary Phone Dial Bluetooth
This sketch reads out the number dialed on a rotary phone dial and connected to a smartphone
via bluetooth (BC127 module) serves as HFP terminal

The rotary dial has two signals: 
            1) turns ON when the wheel is turning
            2) pulses a count based on the number dialed.
debounce is used
and a hook signal (decroche)

 */
//#include "utilcadran.h"
#include "SoftwareSerial.h"

const int  INITPIN = 4; // pin du contact cadran tournant
const int  NUMPIN = 5;  // pin du contact impulsion
const int DECROCHPIN = 6;  // pin du contact décroché
int etat; // etat de la machine à états

SoftwareSerial Serial1(10,11);  // port série pour le debug, RX, TX


void setup(){
  //start serial connections
  Serial1.begin(9600);  // monitor for debug
  Serial.begin(9600); // link to the BC127 module
  // configure the inputs
  pinMode(INITPIN, INPUT_PULLUP); 
  pinMode(NUMPIN, INPUT_PULLUP); 
  pinMode(DECROCHPIN, INPUT_PULLUP); 
  Serial1.println("initialisation");
  etat = 0;
}

void loop(){
  int numero = -1;
  String recucart;
  String envoicart;
  static String numcompo = "";
  uint32_t tdcompo;
  boolean noncompo = true;

  int decroch = digitalRead(DECROCHPIN); // indicateur du décroché à 1 si raccroché
  
  switch (etat)
  {
  case 0:
    recucart = recupdonn(); // données envoyées par la carte BC127
    if (recucart != "") Serial1.println("recu BC127 "+ recucart);
      /* code pour decrochage sans appel entrant on va numeroter*/
    if((recucart == "") && !decroch ) // on vient de decrocher
      {
        etat = 2; // on passe à l etat numerotation
        Serial1.println("decrochage pour numerotation");
      }
    else
      {
        /* code pour gestion des messages entrants*/
        if((recucart != "") && decroch)
        {
          if(recucart.startsWith("CALL +")) // on recoit un appel entrant
          {
            etat = 1;
            Serial1.println(" appel entrant " + recucart); //
          }
        }
        else
        {
          /* caracteres recus non appel entrant*/
          if (recucart != "")  Serial1.println("recu? " + recucart);
          etat = 0;
        }
        
      }
    break; // fin etat 0
  case 1:
    // message entrant
    if(!decroch) // on decroche
    {
      // prise de communication sur appel entrant
      Serial1.println("communication acceptee");
      envoicart = envoidonn("ANSWER\r");
      Serial1.println("recu module "+envoicart);
      etat = 4;
    }
    break;

  case 2: // on numerote apres avoir decroche
    while(noncompo)  // tant que l on a pas atteint la tempo
    {
      tdcompo = millis();
      while((millis()-tdcompo) < 20000) // on ouvre une fenetre de 20s pour composer un premier  numero
          {
            numero = numerotation(INITPIN,NUMPIN);
          // Si numero reste a -1, on ne fait rien, 
            if (numero >= 0) 
            {
              Serial1.println (numero);
              numcompo = numcompo + String(numero);
              numero = -1;
              break; // pas besoin d attendre la fin de tempo
            }
            numero = -1; // remise à négatif
          }
        if((millis()-tdcompo) >= 20000) noncompo = false;
        // numcompo = ""; 
    }
      if (numcompo != "") 
        {
          etat = 3;
          Serial1.println("Numero compose " + numcompo);
        }
      else 
        { 
          etat = 5;  //on arrete en attendant le raccroche
          Serial1.println(" pas de numero compose");
        }

    break;

  case 3: //
    Serial1.println(" attente etablissement comm");
    numcompo = "CALL "+numcompo+"\r";
    envoicart = envoidonn(numcompo);
    Serial1.println("appel");
    numcompo = "";
    etat = 4;
    break;

  case 4: // en cours de communication
    if(decroch) // si raccrochage
    {
      Serial1.println("on coupe la communication");
      envoicart = envoidonn("END\r");
      Serial1.println("recu module "+envoicart);
      etat = 0;
    }
    break;
  
  case 5: // en fin de tempo de numerotation
    Serial1.println("fin tempo, attente raccrochage");
    if(decroch) etat =0;
    break;
  }
}

/*
Sous programmes associés 
*/
int numerotation(int initP,int numP)
{
  int counter = 0; // holds the pulse count for each dial spin
  int currentValue = 0; 
  int countimp = -1;
  unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
  unsigned long debounceDelay = 5;    // the debounce time; increase if the output flickers
  static int lastValue = HIGH;  // holds the last read from the pulse pin.
  
  int initRead = digitalRead (initP); // Is the wheel turning or not?
  
  while (initRead == LOW) {  // If the wheel is turning....
//    Serial.println("composition");
    int newValue = digitalRead (numP); // check the pulse pin.
    if (newValue != lastValue) { // if it's CHANGED from the last read...
      lastDebounceTime = millis(); // save its clock cycle; we need to check it.
    }
    // If enough time has passed (aka, it's not just a "bounce" from the 
    // analog signal)...
    if ((millis() - lastDebounceTime) > debounceDelay) { 
      // and if the current value is DIFFERENT than the one you just read...
      if (currentValue != newValue) { 
        currentValue = newValue; // make them the same.
        if (newValue == 1) { // If you just set it to a 1...
          counter++; // it just finished a pulse, so add one to the counter.
        }
      }
    }
    lastValue = newValue; // Your new value becomes the old one for comparison.
  initRead = digitalRead (initP);
  }

// once the dial returns home and switches the initializing pin OFF,
// you can be sure that there will be no more pulses coming.
// "Counter" is the number dialed. You may now use it for whatever you want.
// This is adjusted for the special case of "0" actually being 10 pulses.
    if (counter > 0) {
      if (counter == 10) countimp = 0;
      else countimp = counter;
        return countimp;
      }

// After you're done using it, reset counter so it can start again on the
// next dial.
    counter = 0;
    countimp = -1; 
return countimp;
}

String recupdonn()  // sous programme de recuperation des donnees envoyees par le module BC127

  {
  static String entbuffer = "";  // We'll use this for capturing data from the module. I'm using
                  //  a String to make it easy to parse.
  String message ="";
    // We need to buffer our incoming serial data...
    if (Serial.available() > 0) entbuffer.concat((char)Serial.read());
    // ...then, we need to check if it's a full line from the serial port
    if (entbuffer.endsWith("\r"))
      { message = entbuffer;
      Serial1.println("Recu du module "+entbuffer);
      entbuffer = "";
      return message;} // Exit avec le message. 
//    Serial.println("Reçu du module"+entbuffer);
  return message; // exit sans rien
  }

String envoidonn(String messenvoi)  // sous programme d'envoi de donnees au module BC127
  {String message;
    if (messenvoi.endsWith("\r"))
      { 
      Serial1.println("Envoi au module "+messenvoi);
      Serial.print(messenvoi);
      uint32_t tdeb = millis();
      while((millis()-tdeb) < 5000)
        {
        message = recupdonn();
        }
      }  
  return message; // exit avec le message en retour du module
  }
