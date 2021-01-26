#ifndef HONEY_H_
#define HONEY_H_

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


typedef struct hive {
    int temperatureEXT;
    unsigned int humidityEXT;
    
    /* pour température intérieure */
    float temperatureINT;
    
    /* pour l'acceleromètre */
    double x , y , z ;   
    
    /*pour température et humidité intérieure */  
    float temperatureINT_SHT ;
    unsigned int humidityINT_SHT ;
    
    int Batterie ; 
    
    int Girouette; 
    unsigned int anenometre; 
    float poids; 
    
    bool has_move; 
    
    int freqMax;
    int dbMax;  
    
    int db_225_285;
    int db_400;
    int db_100;
    int db_1100;
    int db_3000;
} Ruche, *pRuche;

float readtempDS(DS1820& sensor);
int getTemperatureEXT(DHT& sensor);
unsigned int getHumidityEXT(DHT& sensor);
float getTemperatureINT_SHT(SHT21& sensor);
unsigned int getHumidityINT_SHT(SHT21& sensor);
//int getEtatBatterie();
int getAccel(Ruche& ruche);
int getGirouette();
unsigned int get_anemometre();
int getFreq();
float get_poids(HX711& sensor); 


void get_data(Ruche& ruche);


void send_data(void);


void send_data_serial(void);


#endif