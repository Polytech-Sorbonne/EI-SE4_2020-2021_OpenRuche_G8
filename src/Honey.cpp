#include "Honey.h"

//Microphone
AnalogIn microphone(A4);

//DS18B20 Sensor (Temp)
DS1820 ds1820(D3);

//DHT Sensor (Temp + Hum)
DHT dht_ext(D2, DHT22);

//SHT Sensor (Temp + Hum)
I2C i2c(D4,D5); //D4 D5
SHT21 sht(&i2c);

//Battery State
AnalogIn ain(A5); //A2

//Accelerometre
MMA8452 mma8452(D12,A6);

//Girouette
AnalogIn ain2(A1);

//Communication
//Serial pc(USBTX, USBRX);
//Sigfox
Serial sigfox(D1,D0);
//Serial pc(D1,D0); // liaison serie avec le pc pour tester fonctionnement

//Anemometre
DigitalIn anemo(D11);

// Jauge de poid
HX711 poids_sens(D10,D9);

Timer t;

bool flag = false;

Ruche ruche;

const int nbEch = 1024; 
const int frqEch = 7000;
const float PI = 3.14159265;
const int fftSize = (int)nbEch/2;

Mikami::FftReal fft((short)fftSize);

bool full = false;
int cptfft = 0;

float dataIn[nbEch]; 
Mikami::Complex dataOut[fftSize];

//**************************************************** 
/********************MICROPHONE***********************/

// Permet d'échantilloner le signal audio
void echantillonage (void) {

    for(int i = 0; i < fftSize ; i++) {
        dataIn[i] = 4096*microphone.read();
        wait(1.0/(float)frqEch);
    }
    full = true;
}

// Permet de récupérer les fréquences qu'on souhaite étudier
int getFundamental(Mikami::Complex *dataOut){
    int cpt_225_285=0;
    int cpt_400 = 0 ;
    int cpt_100 = 0 ;
    int cpt_1100 = 0 ;
    int cpt_3000 = 0 ;
    
    ruche.db_225_285=0;
    ruche.db_400 = 0 ;
    ruche.db_100 = 0 ;
    ruche.db_1100 = 0 ;
    ruche.db_3000 = 0 ;
    
    int max_freq=0;
    float amplMaxFreq = 0;
    for(int j = 1; j < fftSize; j++){
        
        float t = abs(dataOut[j]);
        if(t > amplMaxFreq){
            max_freq = j;
            amplMaxFreq = t;
        }
        int freqcal = j*(frqEch/nbEch)*2.1;
        
        if  ((225 < freqcal) && ( freqcal < 285)){
            cpt_225_285++;
            ruche.db_225_285 = ruche.db_225_285 + t;
        }
        if  ((390 < freqcal) && ( freqcal < 410)){
            cpt_400++;
            ruche.db_400 = ruche.db_400 + t;
        }
        if  ((90 < freqcal) && ( freqcal < 110)){
            cpt_100++;
            ruche.db_100 = ruche.db_100 + t;
        }
        if  ((1000 < freqcal) && ( freqcal < 1200)){
            cpt_1100++;
            ruche.db_1100 = ruche.db_1100 + t;
        }
        if  ((2900 < freqcal) && ( freqcal < 3100)){
            cpt_3000++;
            ruche.db_3000 = ruche.db_3000 + t;
        } 
    }
    ruche.db_225_285=(int)(ruche.db_225_285/cpt_225_285);
    ruche.db_225_285 = 20*log((float)(abs(ruche.db_225_285/2400)));
    
    ruche.db_400=(int)(ruche.db_400/cpt_400);
    ruche.db_400 =  20*log((float)(abs(ruche.db_400/2400)));
    
    ruche.db_100=(int)(ruche.db_100/cpt_100);
    ruche.db_100 =  20*log((float)(abs(ruche.db_100/2400)));
    
    ruche.db_1100=(int)(ruche.db_1100/cpt_1100);
    ruche.db_1100 =  20*log((float)(abs(ruche.db_1100/2400)));
    
    ruche.db_3000=(int)(ruche.db_3000/cpt_3000);
    ruche.db_3000 =  20*log((float)(abs(ruche.db_3000/2400)));
    
    ruche.freqMax = max_freq*(int)(frqEch/nbEch);
    ruche.dbMax = 20*log((float)(abs(amplMaxFreq/2400)));
    
    if (ruche.db_400<0){ruche.db_400 = 0;}
    if (ruche.db_100<0){ruche.db_100 = 0;}
    if (ruche.db_1100<0){ruche.db_1100 = 0;}
    if (ruche.db_3000<0){ruche.db_3000 = 0;}
    if (ruche.dbMax<0){ruche.dbMax = 0;}

    return ruche.freqMax;
}


int getFreq(){
    int frequence = 0;
    echantillonage();
    
    if (full){
            fft.Execute(dataIn,dataOut);
            frequence = getFundamental(dataOut);
            full = false;
            return (int) (frequence*2.1);
    }
}

//****************************************************
/******************** DS18B20 *************************/

float readtempDS(DS1820& sensor){
    float temp=0;
    int result = 0;
    int flag =0;
    
    while (flag==0)
    
    if (sensor.begin()){
       
       flag=0;
       
       sensor.startConversion();   // start temperature conversion from analog to digital
       wait(1);                  // let DS1820 complete the temperature conversion
       result = sensor.read(temp); // read temperature from DS1820 and perform cyclic redundancy check (CRC)
      
       switch (result) {
           case 0:   // no errors -> 'temp' contains the value of measured temperature 
               return temp;

           case 1:                 // no sensor present -> 'temp' is not updated
                return -1;

           case 2:                 // CRC error -> 'temp' is not updated
               return -2;
       }    
    } 
       return NULL; 
}
/****************************************************/

/********************* DHT22 *************************/

int getTemperatureEXT(DHT& sensor){
  int dht_err = sensor.readData();
  while(dht_err){//detecte si il y a des erreurs
        dht_err = sensor.readData();
  }
        wait(0.5);
        return (int)(sensor.ReadTemperature(CELCIUS)+0.5);
}

unsigned int getHumidityEXT(DHT& sensor){
  int dht_err = sensor.readData();
  while(dht_err){//detecte si il y a des erreurs
        dht_err = sensor.readData();
  }
         wait(0.5);
        return (unsigned int)((sensor.ReadHumidity()+0.5)/10); 
}


/****************************************************/
/*********************** SHT ************************/


float getTemperatureINT_SHT(SHT21& sensor){
    
    return sensor.readTemp();
    }

unsigned int getHumidityINT_SHT(SHT21& sensor){
    
    return (unsigned int)((sensor.readHumidity()+0.5)/10); 
    }


/****************************************************/
/****************Etat Batterie***********************/


int getEtatBatterie(){ 
    float lecture;
    int batterie;
    int etat=0;
    
    lecture = ain.read()*100;
    batterie =(int)lecture; 
       
            if (batterie<= 83 ) {etat=20;}
            if ((83 < batterie)&& (batterie <= 86)) {etat=40;}
            if ((86<batterie)&& (batterie <= 90 )){etat=60;}
            if ((90<batterie)&& (batterie <= 94 )) {etat=80;}
            if (94<batterie){etat=100;}
        

        return (int)((etat+0.5)/10);
    
}
    
/****************************************************/
/******************Accelerometre*********************/


int getAccel(Ruche& ruche){
    mma8452.init();
     // Pour le 1e allumage
    if (!flag){
        Acceleration acceleration = mma8452.readValues();
        ruche.x=acceleration.x;
        ruche.y=acceleration.y;
        ruche.z=acceleration.z;
        flag = true;
        return 0;
    }
    else {
         // sauvegarde de l'ancienne valeur 
        double temp_x,temp_y,temp_z;
        temp_x = ruche.x;
        temp_y = ruche.y;
        temp_z = ruche.z;
        
        //Acquisition de la nouvelle valeur 
        Acceleration acceleration = mma8452.readValues();
        ruche.x=acceleration.x;
        ruche.y=acceleration.y;
        ruche.z=acceleration.z;
        
        if ( abs(temp_x - ruche.x)>0.7 || abs(temp_y - ruche.y)>0.7 || abs(temp_z - ruche.z)>0.7){
            return 1; //ruche a bougée
        }
        
        return 0; //ruche a la meme place
    }
    
    }

/****************************************************/
/****************Etat Girouette***********************/


int getGirouette(){     //0N 1NE 2E 3SE 4S 5SO 6O 7NO
    float lecture;
    int girouette;
    int etat;
    
    lecture = ain2.read()*100;
    girouette =(int)lecture;  
            etat=0;
            if ((84 < girouette)&& (girouette <= 86)) {etat=0;}
            if ((78 <= girouette)&& (girouette <= 79)) {etat=1;}
            if ((88<=girouette)&& (girouette <= 89 )){etat=2;}
            if ((82<=girouette)&& (girouette <= 83 )){etat=3;}
            if ((90<=girouette)&& (girouette <= 91 )){etat=4;}
            if ((75<=girouette)&& (girouette <= 77 )){etat=5;}
            if ((80<=girouette)&& (girouette <= 81 )){etat=6;}
            if ((71<=girouette)&& (girouette <= 73 )){etat=7;}
        return etat;
}

/****************************************************/
/****************Etat Anemometre***********************/

unsigned int get_anemometre(){
    float vitesse=0;
    int cpt=0;
    int num = anemo.read();
    int prev=0;
    t.reset();
    t.start();
    while(t.read()<1){
        if(!prev && num){
            cpt++;
            prev=num;
            num=anemo.read();
        }
        else{
            prev=num;
            num=anemo.read();
        }
    } 
    vitesse=cpt*2.4; 
    t.stop();

    return ((vitesse+0.5)/10); 
}

/*****************************************************/
/*********************POIDS***************************/

float get_poids(HX711& sensor){
    float test;
    sensor.powerUp();
    test = sensor.getGram();
    sensor.powerDown();

    return (float)test*1000;
}

/****************************************************/    
/****************************************************/

//Permet de remlir la structure de données représentant la ruche
void get_data(Ruche& ruche){
    ruche.temperatureINT = readtempDS(ds1820);
    ruche.temperatureEXT = getTemperatureEXT(dht_ext);
    ruche.humidityEXT = getHumidityEXT(dht_ext);
    ruche.temperatureINT_SHT= getTemperatureINT_SHT(sht);
    ruche.humidityINT_SHT= getHumidityINT_SHT(sht);
    ruche.Batterie=getEtatBatterie();
    ruche.Girouette = getGirouette();
    ruche.anenometre= get_anemometre();
    ruche.has_move=getAccel(ruche);
    ruche.poids = get_poids(poids_sens);
    if (ruche.poids < 0 && flag){
        ruche.has_move = 1;
    }
    ruche.freqMax = getFreq(); 
}

/* Envoie en série des données 

void send_data_serial(void){
    pc.printf("Debut acquisition ...\r\n");
    get_data(ruche);
    pc.printf("DHT(T /0.5°): %d \r\nDHT(H /0,02): %u \r\n", ruche.temperatureEXT,ruche.humidityEXT); //GOOD
    pc.printf("SHT(T /0.3°): %.f \r\nSHT(H /0,03): %u \r\n", ruche.temperatureINT_SHT , ruche.humidityINT_SHT); //GOOD
    pc.printf("X[%.2f] Y[%.2f] Z[%.2f] Mouvement :%d \r\n", ruche.x,ruche.y,ruche.z,ruche.has_move);
    pc.printf("Direction Vent %d \r\nVitesse Vent %u\r\n", ruche.Girouette, (unsigned int)ruche.anenometre);
    pc.printf("DS18B20: %.f\r\n" ,ruche.temperatureINT);
    pc.printf("poids: %.2f\r\n" ,ruche.poids);
    pc.printf("Freq Fond: %d\r\n", ruche.freqMax);
    pc.printf("Amplitude: %d\r\n", ruche.dbMax);
    pc.printf("Amplitude 225-285: %d\r\n", ruche.db_225_285);
    pc.printf("Amplitude 400: %d\r\n", ruche.db_400);
    pc.printf("\r\n");
}
*/

// Ajouter le sleep de sigfox ?
void send_data(void){
  
    sigfox.baud(9600);
    get_data(ruche);

    unsigned int bga = 0;
    bga = ((unsigned int)ruche.Batterie << 4 ) | ((unsigned int)ruche.Girouette << 1) | ((unsigned int)ruche.has_move);
    
    unsigned int he_vv = 0;
    he_vv = (unsigned int)(ruche.anenometre << 4 ) | (ruche.humidityEXT);
   
   unsigned int sht_t_DHT_T = 0;
   sht_t_DHT_T= ( (unsigned int)((ruche.temperatureINT_SHT)*10) << 6 ) | ( (int)(ruche.temperatureEXT) );
   
   unsigned int DS_SHT_H = 0;
   DS_SHT_H= ( (unsigned int)((ruche.temperatureINT)*10) << 6 ) | ( (unsigned int)(ruche.humidityINT_SHT) );
   
   sigfox.printf("AT$SF=%02x%02x%04x%04x%04x%02x%02x%02x%02x\r\n", bga,he_vv,sht_t_DHT_T,DS_SHT_H,(unsigned int)ruche.poids,(uint8_t)ruche.db_225_285,(uint8_t)ruche.db_100,(uint8_t)ruche.db_3000,(uint8_t)ruche.db_1100);//,(unsigned int)ruche.db_3000);//,ruche.db_100,ruche.db_400,ruche.db_1100,ruche.db_3000);;// poids ruche.db_225_285);//ruche.db_225_285,ruche.db_100,ruche.db_400,ruche.db_1100,ruche.db_3000);
 
}
