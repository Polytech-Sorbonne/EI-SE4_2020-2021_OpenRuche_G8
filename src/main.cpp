#include "mbed.h"
#include "Honey.h"

//Déclaration de la led qui permet de vérifier que le système s'est bien allumé.
DigitalOut myled(D6);

int main(){ 
    //Allumage de la LED pendant 3 secondes
    myled = 1; 
    wait(3); 
    myled = 0;
   
    while(true){
        
        //Mise en veille de 30 minutes
        WakeUp::set(1800);

        //Envoi des données vers l'interface
        send_data();
    
        deepsleep();

    } 
}
   
 