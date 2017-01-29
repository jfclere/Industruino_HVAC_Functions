// Libraries
#include <Wire.h>
#include <SPI.h>
#include <EthernetIndustruino.h>
#include <DIS.h>
#include <SimpleModbusSlave.h>
#include <BMS.h>
#include <Indio.h>

// Global
int       counter;
int       state;
int       loopcount = 0;
float     EA1_TBX_BASS;
float     EA2_THX_HAUT;

static UC1701 lcd;

static BMS BMS;

#define baud       115200   // tested 9600 to 115200
#define TxEnablePin 9       // INDUSTRUINO RS485
#define   SlaveID 2

#define   HOLDING_REGS_SIZE 8
unsigned int holdingRegs[HOLDING_REGS_SIZE];

// Setup
void setup() 
{
  // Serial for debugging
  Serial.begin( 9600 );
  Serial.println("Starting please wait...");

  modbus_configure(&Serial1, baud, SERIAL_8N2, SlaveID, TxEnablePin, HOLDING_REGS_SIZE, holdingRegs);
  modbus_update_comms(baud, SERIAL_8N2, SlaveID);
 
  // clear LCD
  lcd.begin();
  lcd.clear();

  // Define IO.
  Indio.setADCResolution(14);
  Indio.analogReadMode(1, V10_p);
  Indio.analogReadMode(2, V10_p);
  Indio.analogReadMode(3, V10_p);
  Indio.analogReadMode(4, V10_p);

  Indio.digitalMode(1,INPUT);
  Indio.digitalMode(2,INPUT);
  Indio.digitalMode(3,INPUT);
  Indio.digitalMode(4,INPUT);
  Indio.digitalMode(5,INPUT);
  Indio.digitalMode(6,INPUT);

  Indio.digitalMode(7,OUTPUT);
  Indio.digitalWrite(7,LOW);

  Indio.digitalMode(8,OUTPUT);
  Indio.digitalWrite(8,LOW);
  
  Serial.println("Started...");
 
}

// Loop
void loop() 
{
 
  loopcount++; 
  lcd.setCursor(0, 4); lcd.print(loopcount);

  read_temp();

  // Write the tempeture in the registers.
  holdingRegs[0] = int(EA1_TBX_BASS*100); 
  holdingRegs[1] = int(EA2_THX_HAUT*100); 
  modbus_update();

  // start/stop the channel if required.
  if (holdingRegs[3]) {
    Indio.digitalWrite(7,HIGH);
  } else {
    Indio.digitalWrite(7,LOW);
  }
  if (holdingRegs[4]) {
    Indio.digitalWrite(8,HIGH);
  } else {
    Indio.digitalWrite(8,LOW);
  }
}

// Sondes CTN Type III Precon
void read_temp()
{
        float VTS_PRCN[21] = {20, 40.6, 37.8, 35, 32.3, 29.5, 26.7, 23.9, 21.1, 18.4, 15.6, 12.8, 10, 7.3, 4.5, 1.7, -1.1, -3.9, -6.6, -9.4, -12.2};
        int VTE_PRCN[21] = {20,332,336,340,345,350,356,362,370,379,388,400,412,427,444,464,486,512,543,578,619};
        BMS.T_APPRLN(Indio.analogRead(1)*10, VTE_PRCN, VTS_PRCN, 1);
        // EA1_TBX_BASS = VTS_PRCN[0];
        float vallow = VTS_PRCN[0];
        BMS.T_APPRLN(Indio.analogRead(2)*10, VTE_PRCN, VTS_PRCN, 1);
        // EA2_THX_HAUT = VTS_PRCN[0];
        float valhigh = VTS_PRCN[0];
        if (valhigh != EA2_THX_HAUT || vallow != EA1_TBX_BASS) {
                EA2_THX_HAUT = valhigh;
                EA1_TBX_BASS = vallow;
        	Serial.print("Td     : "); Serial.print(EA1_TBX_BASS);  Serial.println(" oC     ");
        	Serial.print("Tr     : "); Serial.print(EA2_THX_HAUT);  Serial.println(" oC     ");
        }

}
