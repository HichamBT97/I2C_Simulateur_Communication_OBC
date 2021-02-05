// Master program : Arduino Mega

// include the I2C library
#include <Wire.h>

// Define Slave Adress & Size of the data that will receive
#define SLAVE_ADDR  9
#define sizeofdata 20

// Define the Time for repeating loop
#define Time 10000 // repete the process every 10s 

// 50 memory slots reserved for received data
byte data[50] = {0};

// Two variables of char type, first contains the code & the second contains the respons
char code[] = {'V','H','M','S','G'};  // V = Vide ; initial value
                                      // if code = 'H' => One check => response from Slave is 'H' if it is ready
                                      // if code = 'M' => Start Measure => No response from Slave
                                      // if code = 'S' => Get state => Response is 'N' if Not Ready
                                      //                                        or 'R' if Ready
                                      // if code = 'G' => Get Measure => Response is a flux of data
char Resp = 'V';


void setup() {
  // start I2C Bus as Master:
  Wire.begin();

  Serial.begin(9600);
  Serial.println("\t\tI2C Master Demonstration");
}

void loop() {
  delay(Time);  // Wait for 10s before begin communication
  
   // Print to Serial monitor
  Serial.println("Begin Transmission of 'H' code");
  // Start transfer of the 'H' code
  Wire.beginTransmission(SLAVE_ADDR); // start communication with slave
  Wire.write(code[1]);                // send the code byte
  Wire.endTransmission();             // end of Transmission

  delay(40); // wait for 40ms to assure that the bus is available

  Serial.println("End of Transmission and Begin Receive Response");
    // Receive the response from the slave
  Wire.requestFrom(SLAVE_ADDR,1);     // demand for a request
  while (Wire.available()){          
      Resp = Wire.read();               // Read the response from Slave
      Serial.print("End of Receive and the Response is : ");
      Serial.println(Resp);
    }

    delay(40);

   // if Slave is Ready (Resp = 'H') then we send commands
   if(Resp == 'H'){
StartHere:
      // Print to Serial monitor
      Serial.println("Begin Transmission of 'M' code");
      // Start transfer of the code
      Wire.beginTransmission(SLAVE_ADDR); // start communication with slave
      Wire.write(code[2]);                // send the code byte
      Wire.endTransmission();             // end of Transmission
      
      delay(40);  // wait for 40ms to assure that the bus is available
      Serial.println("Transmission of 'M' code is done");
      delay(200); // wait for 200ms to ensure that slave finished the measure
ReAskForMeasur:
      // Print to Serial monitor
      Serial.println("Begin Transmission of 'S' code");
      // Start transfer of the code
      Wire.beginTransmission(SLAVE_ADDR); // start communication with slave
      Wire.write(code[3]);                   // send the code byte
      Wire.endTransmission();             // end of Transmission

      delay(40);

      Serial.println("Receive Response");
      Wire.requestFrom(SLAVE_ADDR,1);     // demand for a request of the state
      while (Wire.available()){          
        Resp = Wire.read();               // Read the response from Slave
       }
       
      if (Resp == 'N'){
        Serial.println("Measure is not Ready");
        Serial.println("Re-asking for the state of measure");
        goto ReAskForMeasur;
      }
   
      if (Resp == 'R')  // if the data is ready then we can ask for data
      { 
        Serial.println("Measure is Ready");
        // start of transfer the code byte
        Serial.println("Begin Transmission of 'G' code");
        Wire.beginTransmission(SLAVE_ADDR); // start communication with slave
        Wire.write(code[4]);                // send the code byte
        Wire.endTransmission();             // end of Transmission

        delay(40);
        
        // Receive the data
        Serial.println("End of Transmission and Begin Receiving data");
        Wire.requestFrom(SLAVE_ADDR,sizeofdata);
        byte i = 0;
        while(Wire.available()){
          data[i] = Wire.read();
          Serial.println(data[i]);
          i++;
        }
       Serial.println("All Measure request has been received");
      }

      else{   // Resp = autre que (N,R) 
        Serial.println("Error! The state received is not defined! The cycle will restart");
        goto StartHere;
      }
   }
}
