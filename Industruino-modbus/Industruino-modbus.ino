// Libraries
#include <SPI.h>
#include <EthernetIndustruino.h>
#include <DIS.h>
#include <SimpleModbusSlave.h>

// Global
int       counter;
int       state;
int       loopcount = 0;

static UC1701 lcd;

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
 
  // clear LCD and write IP.
  lcd.begin();
  lcd.clear();
  Serial.println("Started...");
 
}

// Loop
void loop() 
{
 
  loopcount++; 
  lcd.setCursor(0, 4); lcd.print(loopcount);

  modbus_update();
  for (int i=0; i<HOLDING_REGS_SIZE; i++) {
      Serial.print( "received: "); Serial.print(i);Serial.print(" "); Serial.print(holdingRegs[i]);Serial.println("...");
  }

}
