#ifndef HONEY_H_
#define HONEY_H_

//Include des différentes librairies
#include "mbed.h"
#include "DS1820.h"
#include "DHT.h"
#include "SHT21_ncleee.h"
#include "MMA8452.h"
#include "WakeUp.h"
#include "fftReal.hpp"
#include "HX711.h"
#include <math.h>
#include <stdbool.h>

//Structure définissant les grandeurs de la ruche
typedef struct hive {
    
    
    int temperatureEXT;                             //Température extérieure    (DHT)
    unsigned int humidityEXT;                       //Humidité extérieure       (DHT) 
    
    float temperatureINT;                           //Température intérieure    (DSB18B20)
   
    double x , y , z ;                              //Données accéléromètre 
    
    float temperatureINT_SHT ;                      //Température intérieure    (SHT21)
    unsigned int humidityINT_SHT ;                  //Humidité intérieure       (SHT21)
    
    int Batterie ;                                  //Etat de la batterie
    
    int Girouette;                                  //Direction du vent
    unsigned int anenometre;                        //Vitesse du vent
    float poids;                                    //Poids de la ruche
    
    bool has_move;                                  //Variable mouvement ruche
    
    int freqMax;                                    //Valeur de la fréquence maximale enregistrée
    int dbMax;                                      //Puissance maximale
    
    int db_225_285;                                 //Puissance enregistrée pour des fréquences entre 225 et 285 Hz
    int db_400;                                     //Puissance enregistrée pour une fréquence de 400 Hz
    int db_100;                                     //Puissance enregistrée pour une fréquence de 100 Hz
    int db_1100;                                    //Puissance enregistrée pour une fréquence de 1100 Hz
    int db_3000;                                    //Puissance enregistrée pour une fréquence de 3000 Hz
    
} Ruche, *pRuche;


//Définition des prototypes de fonctions

float readtempDS(DS1820& sensor);                   //Lecture température intérieure   (DS18B20)
int getTemperatureEXT(DHT& sensor);                 //Lecture température extérieure   (DHT)
unsigned int getHumidityEXT(DHT& sensor);           //Lecture humidité extérieure      (DHT)
float getTemperatureINT_SHT(SHT21& sensor);         //Lecture température intérieure   (SHT21)
unsigned int getHumidityINT_SHT(SHT21& sensor);     //Lecture humidité intérieure      (SHT21)
int getEtatBatterie();                              //Lecutre état de la batterie    
int getAccel(Ruche& ruche);                         //Lecture accéléromètre
int getGirouette();                                 //Lecture direction du vent
unsigned int get_anemometre();                      //Lecture vitesse du vent
int getFreq();                                      //Lecture fréquence
float get_poids(HX711& sensor);                     //Lecture du poids

bool get_data(Ruche& ruche);                        //Fonction d'éxecution des différentes lectures de grandeurs            
void send_data(void);                               //Envoi des données par Sigfox
void send_data_serial(void);                        //Envoi des données par communication série


#endif