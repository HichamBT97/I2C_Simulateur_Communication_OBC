/*
 * POSSIBLE CODES FOR COMMUNICATION WITH OLIMEX
// code = 'H' => One check => response from Slave is 'H' if it is ready
// code = 'M' => Start Measure => No response from Slave
// code = 'S' => Get state => Response is 'N' if Not Ready
//                                     or 'R' if Ready
// code = 'G' => Get Measure => Response is a flux of data
*/
#include <Wire.h>
#include <SD.h>
#include<SPI.h>
// declaration des variables pour la communication avec la carte Olimex
#define SLAVE_ADDR  9   // Define Slave Adress
#define sizeofdata 20   // Size of the data that will receive
byte data[50] = {0};    // 50 memory slots reserved for received data                       
char Resp = 'V';  // will contain the respons
char STATE = 'N'; // will contain the state of measure


// variables pour la communication avec la carte memoire
File myFile;
int CS_SD = 53;
int tabsize = 5;
String NomFiles[] = {"test1.txt","test2.txt","test3.txt",   // les noms des fichiers représentant les scénarios
                     "test4.txt","test5.txt"};
String Scenareo[] = {"HELLO\nEND", "HELLO\nGETMESURE\nEND", // 5 scénaios qui seront dans les ficheirs texts
                     "HELLO\nSTARTMESURE\nGETSTATE\nEND",
                     "HELLO\nSTARTMESURE\nGETSTATE\nGETMESURE\nEND",
                     "HELLO\nGETSTATE\nGETMESURE\nEND"};
String *line = (String*)calloc(tabsize,sizeof(String)); // tableau qui stock les lignes lu dans un fichier
String file = "";     // variable qui stock le choix du fichier selectionné par l'utilisateur
enum etats {Wait, SendH, SendM, SendS, SendG};
etats etat = Wait;

void setup() {
  Wire.begin();             // start I2C Bus as Master:
  Serial.begin(9600);       // initialisation du moniteur serie
  pinMode(CS_SD , OUTPUT);  // SD card Chip Select for SPI bus
  SDinitialization();
  for(int i = 0 ; i<5 ; i++){                // 5 represente le nombre de fichier, à changer si le nombre de fichier change
     checkIfexists(NomFiles[i]);             // fonction qui permet de voir si un fichier exist, si oui il sera supprimer
     writeToFile(NomFiles[i] ,Scenareo[i]);  // écrire dans le fichier[i] le scenareo[i]
  }
  Serial.println("\t\tI2C Master Demonstration");
}

void SDinitialization(){ // Fonction pour l'initialisation de la carte memoire 
  Serial.print("Initializing SD card...");
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
}

String ChoixFile(){ // Fonction qui permet de choisir un fichier
  int choixF;
  Serial.println("Choice the scenareo that you want to execute :");
  indice :
  Serial.println("1 : test1.txt\n2 : test2.txt\n3 : test3.txt\n4 : test4.txt\n5 : test5.txt");
  char inv = Serial.read(); // pour eviter le bruit de l espace apres lecture
  while(Serial.available() == 0);
  choixF = (int) (Serial.read())-48;// retranche 48 du code ascii pour la mise en forme du valeur

  if(choixF < 1 || choixF > 5){
    Serial.println("invalide choice...Please make a new choice : ");
    goto indice;
  } else {
    Serial.print("Your choice is : ");
    Serial.println(NomFiles[choixF-1]);
  }
  return NomFiles[choixF-1];
}

void checkIfexists(String file){ // Fonction qui véifier si un fichier existe
  if(SD.exists(file)){
   // Serial.println("the file exists");
    SD.remove(file); // si le fichier existe on le supprime pour reécrire de nouveau
    if(SD.exists(file)){
      Serial.println("the file still exists");
    }
  } else{
      Serial.println("the file was replaced");
  }
}

void writeToFile(String file, String text){ // Fonction permettant d'écrire dans un fichier
  myFile = SD.open(file, FILE_WRITE);
  if (myFile){
    myFile.println(text);
    Serial.print("Writing to ");
    Serial.print(file);
    Serial.println(" is Done.");
  } else{
    Serial.println("Couldn't write to file");
  }
  myFile.close();
}

int readText(String file,String *tab, int tabsize){ // Fonction qui permet de lire un fichier
  myFile = SD.open(file, FILE_READ);
  String received = "";
  char ch;
  int i=0;

  while(myFile.available()){
    ch = myFile.read();
    if(ch == '\n'){
      tab[i] = received;
      received = "";
      if(i > tabsize){
        Serial.println("i indexe passed size of table");
        goto fin;
      }
      i += 1; 
      //Serial.println(i);
    }
    else{
      received += ch;
    }
  }
  fin :
  myFile.close();
  return i; // on retourne le nombre de lignes qui sont dans le fichier
}

void loop() {
  start :
  file = ChoixFile();     // fonction permet de choisir un des fichiers
  int nbrline = readText(file,line,tabsize);
  Serial.println("Scenareo is : ");
  for(int i = 0 ; i < nbrline ; i++){
    Serial.println(line[i]);
  }
  
  if (line[0] == "HELLO"){
    Serial.println("Scenareo is Ok...Begin comminication with Olimex");
    int l = 0;
    for(int i = 0 ; i < nbrline ; i++){
      if(line[i] == "HELLO") etat = SendH;
      else if(line[i] == "STARTMESURE") etat = SendM;
      else if(line[i] == "GETSTATE") etat = SendS;
      else if(line[i] == "GETMESURE") etat = SendG;
      else etat = Wait;
      switch(etat){
        case SendH :
           // Print to Serial monitor
            Serial.println("Begin Transmission of 'H' code");
            reask :
            // Start transfer of the 'H' code
            Wire.beginTransmission(SLAVE_ADDR); // start communication with slave
            Wire.write('H');                    // send the code byte
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

             if(Resp == 'H') Serial.println("Olimex is Ready for commication");
             else {
              Serial.println("Olimex is not Ready..Retransmission of H");
              goto reask;
             }
        break;

        case SendM:
          // Print to Serial monitor
          Serial.println("Begin Transmission of 'M' code");
          // Start transfer of the code
          Wire.beginTransmission(SLAVE_ADDR); // start communication with slave
          Wire.write('M');                // send the code byte
          Wire.endTransmission();             // end of Transmission
          
          delay(40);  // wait for 40ms to assure that the bus is available
          Serial.println("Transmission of 'M' code is done");
          delay(200); // wait for 200ms to ensure that slave finished the measure
        break;

        case SendS:
          ReAskForMeasur:
          // Print to Serial monitor
          Serial.println("Begin Transmission of 'S' code");
          // Start transfer of the code
          Wire.beginTransmission(SLAVE_ADDR); // start communication with slave
          Wire.write('S');                   // send the code byte
          Wire.endTransmission();             // end of Transmission
    
          delay(40);
    
          Serial.println("Receive Response :");
          Wire.requestFrom(SLAVE_ADDR,1);     // demand for a request of the state
          while (Wire.available()){          
            Resp = Wire.read();               // Read the response from Slave
           }
           
          if (Resp == 'N'){
            Serial.println("Measure is not Ready");
            Serial.println("Re-asking for the state of measure");
            goto ReAskForMeasur;
          }
          else if (Resp == 'R'){
            Serial.println("Measure is Ready");
            STATE = 'R';
          }
          else if (Resp == 'E'){
            Serial.println("Error message has been received..Scenareo is not valid..may ask Olimex to start mesure");
            goto start;
          }
          else{
            Serial.println("Response is not valid..check Olimex code");
            goto start;
          }
        break;

        case SendG:
        if (STATE == 'R'){
          STATE == 'N'; // initialise STATE
          // start of transfer the code byte
          Serial.println("Begin Transmission of 'G' code");
          Wire.beginTransmission(SLAVE_ADDR); // start communication with slave
          Wire.write('G');                // send the code byte
          Wire.endTransmission();             // end of Transmission
  
          delay(40);

          // Receive the data
          Serial.println("End of Transmission and Begin Receiving data");
          Wire.requestFrom(SLAVE_ADDR,sizeofdata);
          byte i = 0;
          while(Wire.available()){
            data[i] = Wire.read();
            if(data[i] == 'E'){
              Serial.println("Error message has been received..Scenareo is not valid..may ask for State before");
              goto start;
            }
            Serial.println(data[i]);
            i++;
          }
          Serial.println("All Measure request has been received");
        }
        else Serial.println("Didn't ask for Mesure..Scenareo is not valid");
        break;
      }
    }
  }
}
