/**
 * Romgere Cockpit Library - https://github.com/romgere/romgere_cockpit
 * @author jerome@mestres.fr
 */



#include "EthernetInterface.h"

#define DEBUG_ETHERNET true

#define UDP_TX_DREF_LENGTH 496
#define UDP_TX_PACKET_MAX_SIZE 509
#define XPLANE_DREF_UDP_PACKET_SIZE 509

EthernetInterface::EthernetInterface(unsigned int readPort, unsigned int writePort, IPAddress arduinoIP, uint8_t arduinoMAC[6], IPAddress xplaneIP, bool waitForXPlane) {
    this->IsClassInit  = false;

    for( int i =0; i < MAX_INPUT_DATA_FROM_XPLANE; i++ ){
        this->LastXPlaneDatas[i] = NULL;
    }

    this->XPlaneWritePort = writePort;
    this->XplaneReadPort = readPort;
    this->IsXPlaneAdressInit = false;

    //DHCP : Unknow adress on board initialisation
    if( xplaneIP[0] == 0)
        XPlaneAdress = INADDR_NONE;
    else{
        XPlaneAdress = xplaneIP;
        IsXPlaneAdressInit = true;
    }

    int res = 0;

    //DHCP
	  if ( arduinoIP[0] == 0){

        res = Ethernet.begin(arduinoMAC);

        //DHCP error ! Can't do any other util stuff
        if( res == 0 ){
            for(;;){}
        }
    }
    //Fixed IP
	  else{
        Ethernet.begin( arduinoMAC, arduinoIP);
    }

#ifdef DEBUG_ETHERNET
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
#endif

#ifdef DEBUG_ETHERNET
    while (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
      delay(100);
    }
#endif

    //Start listening on UDP
    res = this->Udp.begin( this->XplaneReadPort);


    //UDP error ! Can't do any other util stuff
    if( res == 0 ){
        for(;;){}
    }
    this->IsClassInit = true;

    //Wait for a first exchange with X-Plane
    if ( waitForXPlane ){


        while ( !this->IsXPlaneAdressInit ){
            this->ReadAllInput();

            if( !this->IsXPlaneAdressInit )
                delay(250);
        }
	}

    Serial.println("EthernetInterface init complete");
}

EthernetInterface::~EthernetInterface(){}

//Read and parse datas received from X-Plane. Return : Number of packet read from x-Plane ()
uint8_t EthernetInterface::ReadAllInput(){

//char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

#ifdef DEBUG_ETHERNET
  Serial.println("ReadAllInput");
#endif

    //Pas initialisée
    if( ! this->IsClassInit )
        return 0;

    int readSize = this->Udp.parsePacket();
      if (readSize){

#ifdef DEBUG_ETHERNET
    Serial.print("Received UDP packet of size ");
    Serial.println(readSize);

    Serial.print("From ");
    IPAddress remote = this->Udp.remoteIP();
    for (int i=0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(this->Udp.remotePort());
#endif
        //First reception : keep X-Plane remote address
        if ( !this->IsXPlaneAdressInit ){
            this->XPlaneAdress = this->Udp.remoteIP();
            this->IsXPlaneAdressInit = true;
        }


        byte buffer[readSize];
        this->Udp.read( buffer, readSize );

#ifdef DEBUG_ETHERNET
  Serial.print("Udp.read: ");
  Serial.println(buffer[0]);
#endif
        //Dats from X-Plane start with "DATA>"
        //ref : http://svglobe.com/arduino/udpdata.html
        if (buffer[0] == 'D' && buffer[1] == 'A' && buffer[2] == 'T' && buffer[3] == 'A'  ){

            //Reset previous datas
            for( int i =0; i < MAX_INPUT_DATA_FROM_XPLANE; i++ ){
                if( this->LastXPlaneDatas[i] != NULL ){
                    delete this->LastXPlaneDatas[i];
                }

                this->LastXPlaneDatas[i] = NULL;
            }


            uint8_t index = 0;
            //Parse received data (By 36 Bytes packet, length of X-Plane data )
            for (int i=5 ; i < readSize; i+=36) {

Serial.println("Here1");
                XPGroupDatas *p = new XPGroupDatas();
Serial.println("Here2");

                //First byte : Data's group
                p->group = buffer[i];

#ifdef DEBUG_ETHERNET
                Serial.print("Ethernet debug : Receive data for group ");
                Serial.print(p->group);
                Serial.print(" with value [");
#endif

                //Datas : table of 8 float value (8 x 4 bytes)
                for (int j=0; j<8; j++){

                    XPNetworkData tmpData;

                    for (int k=0; k<4; k++){
                        tmpData.byteVal[k] = buffer[i + 4 + (j*4) + k];
                    }
                    //Convert 4 byte array to float with XPNetworkData union
                    p->data[j] = tmpData.floatVal;
#ifdef DEBUG_ETHERNET
                    Serial.print(p->data[j]);
                    Serial.print(",");
#endif
                }

#ifdef DEBUG_ETHERNET
                Serial.println("].");
#endif
                this->LastXPlaneDatas[index] = p;

                index++;
            }

            //Return the number of data group read (if 0 no output will be modified)
            return index;

        }

#ifdef DEBUG_ETHERNET
        Serial.println("Ethernet debug : Data received not start with \"DATA>\" : ");
        Serial.println("---------------START------------------");
        for (int i=0; i < readSize; i++)
            Serial.print((char)buffer[i]);
        Serial.println("----------------END-------------------");
#endif
	  }

	  return 0;
}



//Send a key to X-Plane
void EthernetInterface::SendKey( const char* key) {

    if( ! this->IsClassInit )
        return;

    this->Udp.beginPacket( this->XPlaneAdress, this->XPlaneWritePort);
    this->Udp.write("CHAR0");
    this->Udp.write((char)*key);
    this->Udp.write("");
    this->Udp.endPacket();
}


//Send command to X-Plane
void EthernetInterface::SendCommand( const char* cmd) {

    if( ! this->IsClassInit )
        return;

    this->Udp.beginPacket( this->XPlaneAdress, this->XPlaneWritePort);
    this->Udp.write("CMND0");
	  this->Udp.write(cmd) ;
    this->Udp.endPacket();
}


void EthernetInterface::SendDrefCommand( const char *dref, float value){

    if( ! this->IsClassInit )
        return;

    char DataOut[XPLANE_DREF_UDP_PACKET_SIZE + 1];
    char drefLabel[] = "DREF";

    XPNetworkData tmpData;
    tmpData.floatVal = value;

    strcpy(DataOut, drefLabel);
    DataOut[4] = 0; //zero terminate label

    for (int copyCounter = 0; copyCounter<4; copyCounter++) { //4 bytes of float value
      DataOut[5 + copyCounter] = tmpData.byteVal[copyCounter];
    }
    
    strcpy( &DataOut[9], "sim/");
    strcpy( &DataOut[13], dref);

    int drefTerminationPoint = strlen(dref) + 13;
    DataOut[drefTerminationPoint] = 0; //terminate dref_path value with 0
    memset(&DataOut[drefTerminationPoint + 1], char(32), XPLANE_DREF_UDP_PACKET_SIZE - drefTerminationPoint);

    DataOut[XPLANE_DREF_UDP_PACKET_SIZE - 1] = 0;

#ifdef DEBUG_ETHERNET
    Serial.print("Send DREF command: ");
    for (int k=0;k<UDP_TX_DREF_LENGTH;k++)
    {
      if (isPrintable(DataOut[k]))
      {
        Serial.print(DataOut[k]);
      }
      else
      {
        Serial.print("\\0x");
        Serial.print(DataOut[k], HEX);
      }
    }
    Serial.println("<END>");
#endif

    this->Udp.beginPacket( this->XPlaneAdress, this->XPlaneWritePort);
    this->Udp.write(DataOut, XPLANE_DREF_UDP_PACKET_SIZE);
    this->Udp.endPacket();
}

//Get a datas received for a given group number
XPGroupDatas* EthernetInterface::GetData( float group ){

    for( int i =0; i < MAX_INPUT_DATA_FROM_XPLANE; i++ ){
        if( this->LastXPlaneDatas[i] != NULL &&  this->LastXPlaneDatas[i]->group == group ){
            return this->LastXPlaneDatas[i];
        }
    }

    return NULL;
}
