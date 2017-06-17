// Ladyada's logger modified by Bill Greiman to use the SdFat library

// this is a generic logger that does checksum testing so the data written should be always good
// Assumes a sirf III chipset logger attached to pin 2 and 3

#include <SD.h>


#define SERIAL_SET "$PSRF100,01,4800,08,01,00*0E\r\n"

// GGA-Global Positioning System Fixed Data, message 103,00

#define GGA_ON "$PSRF103,00,00,01,01*25\r\n"
#define GGA_OFF "$PSRF103,00,00,00,01*24\r\n"

// GLL-Geographic Position-Latitude/Longitude, message 103,01

#define GLL_ON "$PSRF103,01,00,01,01*26\r\n"
#define GLL_OFF "$PSRF103,01,00,00,01*27\r\n"

// GSA-GNSS DOP and Active Satellites, message 103,02

#define GSA_ON "$PSRF103,02,00,01,01*27\r\n"
#define GSA_OFF "$PSRF103,02,00,00,01*26\r\n"

// GSV-GNSS Satellites in View, message 103,03

#define GSV_ON "$PSRF103,03,00,01,01*26\r\n"
#define GSV_OFF "$PSRF103,03,00,00,01*27\r\n"

// RMC-Recommended Minimum Specific GNSS Data, message 103,04

#define RMC_ON "$PSRF103,04,00,01,01*21\r\n"
#define RMC_OFF "$PSRF103,04,00,00,01*20\r\n"

// VTG-Course Over Ground and Ground Speed, message 103,05

#define VTG_ON "$PSRF103,05,00,01,01*20\r\n"
#define VTG_OFF "$PSRF103,05,00,00,01*21\r\n"

// Switch Development Data Messages On/Off, message 105
#define LOG_DDM 1
#define DDM_ON "$PSRF105,01*3E\r\n"
#define DDM_OFF "$PSRF105,00*3F\r\n"

#define USE_WAAS 0 // useful in US, but slower fix
#define WAAS_ON "$PSRF151,01*3F\r\n" // the command for turning on WAAS
#define WAAS_OFF "$PSRF151,00*3E\r\n" // the command for turning off WAAS

#include <SoftwareSerial.h>

// what to log
#define LOG_RMC 1 // RMC-Recommended Minimum Specific GNSS Data, message 103,04
#define LOG_GGA 0 // GGA-Global Positioning System Fixed Data, message 103,00
#define LOG_GLL 0 // GLL-Geographic Position-Latitude/Longitude, message 103,01
#define LOG_GSA 0 // GSA-GNSS DOP and Active Satellites, message 103,02
#define LOG_GSV 0 // GSV-GNSS Satellites in View, message 103,03
#define LOG_VTG 0 // VTG-Course Over Ground and Ground Speed, message 103,05

// Use pins 2 and 3 to talk to the GPS. 2 is the TX pin, 3 is the RX pin
SoftwareSerial gpsSerial = SoftwareSerial(2, 3);

// Set the GPSRATE to the baud rate of the GPS module. Most are 4800
// but some are 38400 or other. Check the datasheet!
#define GPSRATE 4800

// Set the Serial communication baud rate of the computer
#define SERIALRATE 19200

// Set the pins used
#define powerPin 4
#define led1Pin 5
#define led2Pin 6
#define chipSelect 10


#define BUFFSIZE 128
char buffer[BUFFSIZE];
uint8_t bufferidx = 0;

uint8_t i;
File logfile;
bool TERMINALMODE = 1; // activate terminal mode at startup, not GPS logging



// blink out an error code
void error(uint8_t errno) {
/*
if (SD.errorCode()) {
putstring("SD error: ");
Serial.print(card.errorCode(), HEX);
Serial.print(',');
Serial.println(card.errorData(), HEX);
}
*/
  while(1) {
    for (i=0; i<errno; i++) {
      digitalWrite(led1Pin, HIGH);
      digitalWrite(led2Pin, HIGH);
      delay(100);
      digitalWrite(led1Pin, LOW);
      digitalWrite(led2Pin, LOW);
      delay(100);
    }
    for (; i<10; i++) {
      delay(200);
    }
  }
}

void setup() {

  Serial.begin(SERIALRATE);
  Serial.println("\r\nGPSlogger");
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, HIGH);//don't power GPS on

  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card init. failed!");
    error(1);
  }
  else {
    Serial.println("Card init. ok");
  }
  
  
  // waiting for a serial connection
  while (millis()<10000)
  {
    if (Serial.available()>0)
    {
      if (Serial.read()=='\r')
      {
        TERMINALMODE = 1;
        break;
      }
    }
   TERMINALMODE = 0; 
  }
  if (TERMINALMODE)
  {
    Serial.println("Device is set to terminal mode for data management");
    Serial.println("Press 'p' to print all the traces");
    Serial.println("Press 'e' to erase all the traces");
  }
  else
  {
    Serial.println("Device is set to GPS data logger");
    
    digitalWrite(powerPin, LOW); //turn on GPS
    delay(800);
    
    //find the filename to write data into, skip existing files
    strcpy(buffer, "GPSLOG00.TXT");
    for (i = 0; i < 100; i++) {
      buffer[6] = '0' + i/10;
      buffer[7] = '0' + i%10;
      // create if does not exist, do not open existing, write, sync after write
      if (! SD.exists(buffer)) {
        break;
      }
    }
    // current filename is buffer
    
    logfile = SD.open(buffer, FILE_WRITE);
    if( ! logfile ) {
      Serial.print("Couldnt create "); Serial.println(buffer);
      error(3);
    }
    Serial.print("Writing to "); Serial.println(buffer);
  
    // connect to the GPS at the desired rate
    gpsSerial.begin(GPSRATE);
  
    Serial.println("Ready!");
  
    gpsSerial.print(SERIAL_SET);
    delay(250);

    #if (LOG_DDM == 1)
         gpsSerial.print(DDM_ON);
    #else
         gpsSerial.print(DDM_OFF);
    #endif
      delay(250);
    #if (LOG_GGA == 1)
        gpsSerial.print(GGA_ON);
    #else
        gpsSerial.print(GGA_OFF);
    #endif
      delay(250);
    #if (LOG_GLL == 1)
        gpsSerial.print(GLL_ON);
    #else
        gpsSerial.print(GLL_OFF);
    #endif
      delay(250);
    #if (LOG_GSA == 1)
        gpsSerial.print(GSA_ON);
    #else
        gpsSerial.print(GSA_OFF);
    #endif
      delay(250);
    #if (LOG_GSV == 1)
        gpsSerial.print(GSV_ON);
    #else
        gpsSerial.print(GSV_OFF);
    #endif
      delay(250);
    #if (LOG_RMC == 1)
        gpsSerial.print(RMC_ON);
    #else
        gpsSerial.print(RMC_OFF);
    #endif
      delay(250);
    
    #if (LOG_VTG == 1)
        gpsSerial.print(VTG_ON);
    #else
        gpsSerial.print(VTG_OFF);
    #endif
      delay(250);
    
    #if (USE_WAAS == 1)
        gpsSerial.print(WAAS_ON);
    #else
        gpsSerial.print(WAAS_OFF);
    #endif
      delay(250);
  }
}      
    
void loop() {

  if (TERMINALMODE)
  {
      if (Serial.available()>0)
      {
        char cc = Serial.read();
        if (cc=='p')
        {
            Serial.println("printing all traces");
            char filename[13];
            strcpy(filename, "GPSLOG00.TXT");
            for (i = 0; i < 100; i++) {
            
               filename[6] = '0' + i/10;
               filename[7] = '0' + i%10;        
    
            // create if does not exist, do not open existing, write, sync after write
               if (SD.exists(filename))
               {
                 Serial.println(filename);
                 File dataFile = SD.open(filename,FILE_READ);
                 // if the file is available, write to it:
                 if (dataFile) {
                   while (dataFile.available())
                   {
                     Serial.write(dataFile.read());
                   }
                   dataFile.close();
                 }           
               }
             }
             Serial.println("Done!");
         
        }
        
        if (cc=='e')
        {
            Serial.println("erasing all traces");
            char filename[13];
            strcpy(filename, "GPSLOG00.TXT");
            for (i = 0; i < 100; i++) {
            
               filename[6] = '0' + i/10;
               filename[7] = '0' + i%10;        
    
            // create if does not exist, do not open existing, write, sync after write
               if (SD.exists(filename))
               {
                 SD.remove(filename);
               }
             }
             Serial.println("Done!");
        }   
        
      }
  
  }
  else
  {
      
      if (Serial.available()>0)
      {
        if (Serial.read()=='\r')
        {
          logfile.close();
          TERMINALMODE = 1;
          Serial.println("Device is set to terminal mode for data management");
          Serial.println("Press 'p' to print all the traces");
          Serial.println("Press 'e' to erase all the traces");
          return;
        }
      }
    
      char c;
      uint8_t sum;
    
      // read one 'line'
      if (gpsSerial.available()) {
        c = gpsSerial.read();
    
        if (bufferidx == 0) {
          while (c != '$')
            c = gpsSerial.read(); // wait till we get a $
        }
      buffer[bufferidx] = c;
  
      if (c == '\n') {
        buffer[bufferidx+1] = 0; // terminate it
  

      // rad. lets log it!
        
      Serial.print(buffer); //first, write it to the serial monitor

        

      // Bill Greiman - need to write bufferidx + 1 bytes to getCR/LF
      bufferidx++;

      digitalWrite(led2Pin, HIGH); // Turn on LED 2 (indicates write to SD)

      //add a very simple check before writing
      if (buffer[1]=='G'){

        logfile.write((uint8_t *) buffer, bufferidx); //write the string to the SD file
        logfile.flush();
      }

      digitalWrite(led2Pin, LOW); //turn off LED2 (write to SD is finished)

      bufferidx = 0; //reset buffer pointer

      
      
      return;

        
      }
      if (bufferidx<BUFFSIZE-2) bufferidx++;

  
    }  
    
  }


}


/* End code */
