#include <Arduino.h>

/*
Rotary Phone Dial Bluetooth
This sketch reads out the number dialed on a rotary phone dial and connected to a smartphone
via bluetooth (BT201 module) serves as HFP terminal

The rotary dial has two signals: 
            1) turns ON when the wheel is turning
            2) pulses a count based on the number dialed.
debounce is used
and a hook signal (decroche)

 */
#include "utilcadran.h"
const int  INITPIN = PA5; // pin du contact cadran tournant
const int  NUMPIN = PA6;  // pin du contact impulsion
const int DECROCHPIN = PA7;  // pin du contact décroché
int etat; // etat de la machine à états

HardwareSerial Serial2(PA3,PA2);  // port série debug, le Serial est lié au BT201


void setup(){
  //start serial connections
  Serial2.begin(9600);  // monitor for debug
  Serial.begin(115200); // link to the BT201 module
  // configure the inputs
  pinMode(INITPIN, INPUT_PULLUP); 
  pinMode(NUMPIN, INPUT_PULLUP); 
  pinMode(DECROCHPIN, INPUT_PULLUP); 
  Serial2.println("initialisation");
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
    recucart = recupdonn(); // données envoyées par la carte BT201
    if (recucart != "") Serial2.println("recu BT201 "+ recucart);
      /* code pour decrochage sans appel entrant on va numeroter*/
    if((recucart == "") && !decroch ) // on vient de decrocher
      {
        etat = 2; // on passe à l etat numerotation
        Serial2.println("decrochage pour numerotation");
      }
    else
      {
        /* code pour gestion des messages entrants*/
        if((recucart != "") && decroch)
        {
          if(recucart.startsWith("TT+")) // on recoit un appel entrant
          {
            etat = 1;
            Serial2.println(" appel entrant " + recucart); //
          }
        }
        else
        {
          /* caracteres recus non appel entrant*/
          if (recucart != "")  Serial2.println("recu? " + recucart);
          etat = 0;
        }
        
      }
    break; // fin etat 0
  case 1:
    // message entrant
    if(!decroch) // on decroche
    {
      // prise de communication sur appel entrant
      Serial2.println("communication acceptee");
      envoicart = envoidonn("AT+BA04\r\n");
      Serial2.println("recu module "+envoicart);
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
              Serial2.println (numero);
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
          Serial2.println("Numero compose " + numcompo);
        }
      else 
        { 
          etat = 5;  //on arrete en attendant le raccroche
          Serial2.println(" pas de numero compose");
        }

    break;

  case 3: //
    Serial2.println(" attente etablissement comm");
    numcompo = "AT+BT"+numcompo+"\r\n";
    envoicart = envoidonn(numcompo);
    Serial2.println("appel");
    numcompo = "";
    etat = 4;
    break;

  case 4: // en cours de communication
    if(decroch) // si raccrochage
    {
      Serial2.println("on coupe la communication");
      envoicart = envoidonn("AT+BA03\r\n");
      Serial2.println("recu module "+envoicart);
      etat = 0;
    }
    break;
  
  case 5: // en fin de tempo de numerotation
    Serial2.println("fin tempo, attente raccrochage");
    if(decroch) etat =0;
    break;
  }
}


