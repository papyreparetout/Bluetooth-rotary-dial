#ifndef utilcadran_h
#define utilcadran_h
#include <Arduino.h>
/*
numerotation avec le cadran telephonique
le premier parametre est la pin de connexion du contact debut numerotation
le deuxième est le contact des impulsions
*/
int numerotation(int initP,int numP);

//
//recuperation des messages envoyés par le module BT201
//
String recupdonn();

//
// envoi de message au module BT201
//
String envoidonn(String message);

#endif
