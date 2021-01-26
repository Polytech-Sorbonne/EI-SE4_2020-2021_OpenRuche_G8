#include "mbed.h"
#include "Honey.h"


int main(){ 
   
    while(true){
      
     //WakeUp::set(1800);
     WakeUp::set(900);

   send_data();
    
    deepsleep();
  // wait(2);

    } 
}
   
 