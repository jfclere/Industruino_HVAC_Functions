// Libraries
#include <SPI.h>
#include <EthernetIndustruino.h>
#include <DIS.h>
#define W5500_ETHERNET_SHIELD

// Defines
// Used for connectivity parameters
#define ENDPOINT "192.168.1.35"
#define LOGIN    " "
#define PASSCODE " "
#define PORT     61613
#define TOPIC    "/topic/PITopic"
#define QUEUE    "/queue/Industruino"
#define ZEROBYTE (uint8_t)0x00

// Constants for STOMP
const int DISCONNECTED = 0;
const int CONNECTING = 1;
const int CONNECTED = 2;
const int WAITING = 3;
const int SUBSCRIBING = 4;
const int SUBSCRIBED = 5;
const int SENDING = 6;
const int SENT = 7;
const int DISCONNECTING = 8;
const int STOP = 9;

// Global
int       counter;
int       state;
String    session;
EthernetClient client; //Ethernet client mode
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 21);
static UC1701 lcd;

// Setup
void setup() 
{
  // Initially disconnected
  state = DISCONNECTED;

  //Setup of Ethernet module chip select lines
  pinMode(10, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(10, LOW);
  digitalWrite(6, HIGH);
  digitalWrite(4, HIGH);

  delay(5000); //wait 5 seconds to make sure Ethernet is running.
  
  // Serial for debugging
  Serial.begin( 9600 );
 
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {

    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);

  }
  // give the Ethernet shield a second to initialize:
  delay(1000);

  // clear LCD and write IP.
  lcd.begin();
  lcd.clear();
  lcd.setCursor(0, 1); lcd.print(Ethernet.localIP());
  Serial.println("starting..");
 
}

// Loop
void loop() 
{
  // int    start;
  String body;
  String frame;
  String response;
  
  // If disconnected
  if( state == DISCONNECTED )
  {
    // Connect to server
    if( client.connect( ENDPOINT, PORT ) )
    {
      // Set connecting state
      state = CONNECTING;
      Serial.println( "Connecting..." );
    } else {
      // Problem connecting
      Serial.println( "Not connected" );
    }    
  // Connected to server
  } else if( state == CONNECTING ) {
    // Initiate STOMP connection
    client.println( "CONNECT" );
    client.println( "accept-version:1.2" );
    client.print( "host:" );
    client.println( ENDPOINT );
    client.print( "login:" );
    client.println( LOGIN );
    client.print( "passcode:" );
    client.println( PASSCODE );
    client.println();
    client.write( ZEROBYTE );
    
    // Waiting for response  
    Serial.println( "Connect header sent..." );
    state = WAITING;
  // Connected to STOMP broker
  } else if( state == CONNECTED ) {
    // Subscribe to specified destination
    client.println( "SUBSCRIBE" );
    client.print( "id:" );
    client.println( session );
    client.print( "destination:" );
    client.println( QUEUE );
    client.println( "receipt:subscribed" );
    client.println();
    client.write( ZEROBYTE);
    
    // Subscribing to queue
    Serial.println( "Subscribing to queue..." );
    state = SUBSCRIBING;   
  } else if( state == SUBSCRIBED ) {
    counter = counter + 1;
    body = "{\"deviceID\": \"Industruino\", \"text\": \"Hello\"}";
    
    client.println( "SEND" );
    client.print( "destination:" );
    client.println( TOPIC );
    client.println( "content-type:text/plain" );
    client.print( "content-length:" );
    client.println( body.length() );
    client.print( "receipt:message-" );
    client.println( counter );
    client.println();
    client.print( body );
    client.write( ZEROBYTE);
    
    state = SENDING;
    Serial.println( "Sending message..." );
  } else if( state == SENT ) {
    body = String( counter );
    
    client.println( "DISCONNECT" );
    client.print( "receipt:disconnect-" );
    client.println( body );
    client.println();
    client.write( ZEROBYTE);
    
    state = DISCONNECTING;
    lcd.setCursor(0, 7); lcd.clearLine(); lcd.print("Disconnecting..." );
    Serial.println( "Disconnecting..." );
  }
  
  // Incoming data from broker
  if( client.available() > 0 ) 
  {
    response = client.readStringUntil( 0x0 );
    
    Serial.print( "*** Response Start (" );
    Serial.print( response.length() );
    Serial.println( ") ***" );
    Serial.println( response );
    Serial.println( "*** Response End ***" );
    lcd.setCursor(0, 5); lcd.clearLine(); lcd.print("Received: "); lcd.print(response.length());
    
    if( response.length() > 0 )
    {
      frame = getValue( response, 0, "\n" );
      lcd.setCursor(0, 6); lcd.clearLine(); lcd.print(frame);
      
      if( frame == "CONNECTED" )
      {
        // Get session for later reference
        session = getHeader( response, "session" );
        Serial.print( "Session: " );
        Serial.println( session );
        
        // Set state
        state = CONNECTED;
        Serial.println( "Connected." );      
      } else if( frame == "MESSAGE" ) {
        body = getValue( response, 1, "\n\n" );
        
        Serial.print( "Message: " );
        Serial.println( body );  
      } else if( frame == "RECEIPT" ) {
        if( state == SUBSCRIBING ) 
        {
          state = SUBSCRIBED;
          Serial.println( "Subscribed." );
          lcd.setCursor(0, 7); lcd.clearLine(); lcd.print("Subscribed...");
        } else if( state == SENDING ) {
          state = SENT;
          Serial.println( "Sent." );  
          
          lcd.setCursor(0, 7); lcd.clearLine(); lcd.print("Sent...");
          delay( 1000 );
        } else if( state == DISCONNECTING ) {
          Serial.println( "Disconnected." );
          
          client.stop();
          Serial.println( "Connection closed." );
          lcd.setCursor(0, 7); lcd.clearLine(); lcd.print("DONE");
          state = STOP;  
        }
      }
    }

    // Read last null byte
    client.read();
  }
}

// Count the number of parts in a string
// Used to help replace lack of split
int count( String content, String delimeter )
{
  int count = 0;
  int end;
  int start = 0;
 
  // Count occurances of delimeter
  do {
    end = content.indexOf( delimeter, start );
    start = end + 1;
    count = count + 1;
  } while( end > 0 );
  
  // Return occurance count
  return count;
}

// Reads response for specific header
// Extracts and returns header value
String getHeader( String content, String header )
{
  int    parts;
  int    start;
  String line;  
  String prefix;
  String result;
  
  // How many lines in response
  parts = count( content, "\n" );
  
  // Start on line after frame line
  // Look for header prefix match
  for( int p = 1; p < parts; p++ )
  {
    // Header line
    // Split into parts
    line = getValue( content, p, "\n" );
    prefix = getValue( line, 0, ":" );
    
    // If prefix matches
    if( prefix == header )
    {
      // Get value for header
      start = line.indexOf( ":" ) + 1;
      result = line.substring( start );
      break;
    }  
  }
  
  // Return result
  return result;
}

// Get a specific section of a string
// Based on delimeters
// Used to replace lack of split
String getValue( String content, int part, String delimeter )
{
  int    end;
  int    start = 0;
  String result;

  // Iterate past unwanted values
  for( int count = 0; count < part; count++ )
  {
    end = content.indexOf( delimeter, start );
    start = end + delimeter.length();
  }
  
  // Get next occurance of delimeter
  // May return -1 if not found
  end = content.indexOf( delimeter, start );
  
  // If no more occurances
  if( end == -1 )
  {
    // Must be last value in content
    // Parse out remainder
    result = content.substring( start );
  } else {
    // Otherwise parse out segment of content
    result = content.substring( start, end );
  }
  
  // Clean off white space
  result.trim();
  
  // Return resulting content
  return result;
}
