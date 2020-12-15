
#include <Arduino.h>
#include "utilcadran.h"

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

String recupdonn()  // sous programme de recuperation des donnees envoyees par le module BT201

	{
	static String entbuffer = "";  // We'll use this for capturing data from the module. I'm using
                  //  a String to make it easy to parse.
	String message ="";
		// We need to buffer our incoming serial data...
		if (Serial2.available() > 0) entbuffer.concat((char)Serial2.read());
		// ...then, we need to check if it's a full line from the serial port
		if (entbuffer.endsWith("\r\n"))
			{ message = entbuffer;
			Serial.println("Recu du module "+entbuffer);
			entbuffer = "";
			return message;} // Exit avec le message. 
//		Serial.println("Reçu du module"+entbuffer);
	return message; // exit sans rien
	}

String envoidonn(String messenvoi)  // sous programme d'envoi de donnees au module BT201
	{String message;
		if (messenvoi.endsWith("\r\n"))
			{ 
			Serial.println("Envoi au module "+messenvoi);
			Serial2.print(messenvoi);
			uint32_t tdeb = millis();
			while((millis()-tdeb) < 5000)
				{
				message = recupdonn();
				}
			}  
	return message; // exit avec le message en retour du module
	}


