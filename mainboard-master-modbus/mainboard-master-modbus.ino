#include <ModbusMaster.h>
//#define MESSAGE_LENGTH 3

#define CMD_SETIO "S00\0"
#define CMD_MEASURE "M00\0"

// instantiate ModbusMaster object
ModbusMaster node_1, node_2, node_3, node_4, node_5, node_6;

struct gasParams{
  int intHF;
  int intCO;
  int intTemp;
}; 

struct powerMeterParams{
  int intVolt;
  int intAmp;
  long lnWatt;
  long lnkWh;
};

gasParams gasSensor_1, gasSensor_2, gasSensor_3;
powerMeterParams powerMeter_1, powerMeter_2, powerMeter_3;

//String serverInputString = "";         // a String to hold incoming data
bool serverStringComplete = false;  // whether the string is complete
char serverInputString[15]= {0};
char chCount = 0;
char buffer[250] = "";

void setup()
{
  pinMode(16, INPUT);    // sets the digital pin 13 as output
  pinMode(17, INPUT);    // sets the digital pin 13 as output
  pinMode(18, INPUT);    // sets the digital pin 13 as output
  pinMode(19, INPUT);    // sets the digital pin 13 as output
  
  pinMode(10, OUTPUT);    // sets the digital pin 13 as output
  pinMode(11, OUTPUT);    // sets the digital pin 13 as output
  pinMode(12, OUTPUT);    // sets the digital pin 13 as output
  pinMode(13, OUTPUT);    // sets the digital pin 13 as output
  
  // use Serial (port 0); initialize Modbus communication baud rate
  Serial.begin(115200);
  Serial1.begin(9600);
  Serial2.begin(9600);
  // communicate with Modbus slave ID[] over Serial (port 1)
  node_1.begin(1, Serial1);
  node_2.begin(2, Serial1);
  node_3.begin(3, Serial1);
  node_4.begin(4, Serial1);
  node_5.begin(5, Serial1);
  node_6.begin(6, Serial1);
  Serial2.println("Hello World");
}


void loop()
{
    process();
}

bool process(){
   if(serverStringComplete){
    Serial2.println(serverInputString);

    char *strSplit = strtok(serverInputString, ":,\r\n");

     if(strncmp(strSplit, CMD_MEASURE, strlen(CMD_MEASURE)) == 0){
      gasModbusRead(node_1, &gasSensor_1);
      gasModbusRead(node_2, &gasSensor_2);
      gasModbusRead(node_3, &gasSensor_3);
      powerMeterModbusRead(node_4, &powerMeter_1);
      powerMeterModbusRead(node_5, &powerMeter_2);
      powerMeterModbusRead(node_6, &powerMeter_3);
  
//      sprintf(buffer, "%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%1d,%1d,%1d,%1d", 
//        (int)gasSensor_1.fltHF * 100, (int)gasSensor_2.fltHF * 100, 0,
//        (int)gasSensor_1.fltCO * 100, (int)gasSensor_2.fltCO * 100, 0,
//        (int)gasSensor_1.fltTemp * 100, (int)gasSensor_2.fltTemp * 100, 0,
//        digitalRead(16), digitalRead(17), digitalRead(18), digitalRead(19));

      sprintf(buffer, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld,%1d,%1d,%1d,%1d", 
        gasSensor_1.intHF, gasSensor_2.intHF, gasSensor_3.intHF,
        gasSensor_1.intCO, gasSensor_2.intCO, gasSensor_3.intCO,
        gasSensor_1.intTemp, gasSensor_2.intTemp, gasSensor_3.intTemp,
        powerMeter_1.intVolt, powerMeter_2.intVolt, powerMeter_3.intVolt,
        powerMeter_1.intAmp, powerMeter_2.intAmp, powerMeter_3.intAmp,
        powerMeter_1.lnWatt, powerMeter_2.lnWatt, powerMeter_3.lnWatt,
        digitalRead(16), digitalRead(17), digitalRead(18), digitalRead(19));
      
      Serial.println(buffer);
      Serial2.println(buffer);
      chCount = 0;
      serverStringComplete = false;
      return true;
     }
    
    if(strncmp(strSplit, CMD_SETIO, strlen(CMD_SETIO)) == 0){
      strSplit = strtok(0, ":,\r\n");
      digitalWrite(10, strSplit[0] & 0x0F);
      digitalWrite(11, strSplit[1] & 0x0F);
      digitalWrite(12, strSplit[2] & 0x0F);
      digitalWrite(13, strSplit[3] & 0x0F);
      chCount = 0;
      serverStringComplete = false;
      return true;
    }
    
       //parsing command fail
       Serial.println(F(":A,String received is invalid"));
       serverStringComplete = false;
       chCount = 0;
       return false;
  }
}

void serialEvent() {
  if (Serial.available()) {
    char inChar = (char)Serial.read();
    serverInputString[chCount] = inChar;
     chCount++;
    if (inChar == '\r') {
//      Serial2.println(serverInputString);
      serverStringComplete = true;
      serverInputString[chCount] = 0;
    }
    if(chCount > 15){
      chCount = 0;
      serverStringComplete = false;
    }
  }
}

bool gasModbusRead(ModbusMaster slaveNode, gasParams *ptr_gasParams){
  uint8_t responseCode;
  
  responseCode = slaveNode.readInputRegisters(0, 3);

  if (responseCode == slaveNode.ku8MBSuccess){
      ptr_gasParams->intHF = slaveNode.getResponseBuffer(0);
      ptr_gasParams->intCO = slaveNode.getResponseBuffer(1);
      ptr_gasParams->intTemp = slaveNode.getResponseBuffer(2);
    delay(10);
    return true;
  }
  return false;
}

bool powerMeterModbusRead(ModbusMaster slaveNode, powerMeterParams *ptr_powerMeterParams){
  uint8_t responseCode;
  
  responseCode = slaveNode.readHoldingRegisters(0, 6);

    if (responseCode == slaveNode.ku8MBSuccess){
      ptr_powerMeterParams->intVolt = slaveNode.getResponseBuffer(0);
      ptr_powerMeterParams->intAmp = slaveNode.getResponseBuffer(1);
      ptr_powerMeterParams->lnWatt = (slaveNode.getResponseBuffer(2) << 8) | slaveNode.getResponseBuffer(3);
      ptr_powerMeterParams->lnkWh = (slaveNode.getResponseBuffer(4) << 8) | slaveNode.getResponseBuffer(5);
    delay(10);
    return true;
  }
  return false;
}
