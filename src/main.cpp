#include <Arduino.h>

// MCP2515 CAN Send and Recieve Test

#include <mcp_can.h> //https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>

unsigned long rxId;
byte len;
byte rxBuf[8];
uint8_t txBuf0[8];
uint8_t txBuf1[8];
long Pre_millis;

MCP_CAN CAN0(10);// CAN0 CS: pin 10
MCP_CAN CAN1(9); // CAN1 CS: pin 9

int16_t target_current = 1000;

int16_t fdb_angle = 0;
int16_t fdb_rpm = 100;
int16_t fdb_current = 2000;
int16_t fdb_temp = 20;

void setup()
{
  Serial.begin(115200);

  // init CAN0 bus, baudrate: 250kbps@8MHz
  if(CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK){
    Serial.println("CAN0: Init OK!");
    CAN0.setMode(MCP_NORMAL);
  } else{ 
    Serial.println("CAN0: Init Fail!");
  }

  // init CAN1 bus, baudrate: 250kbps@8MHz
  if(CAN1.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK){
    Serial.println("CAN1: Init OK!");
    CAN1.setMode(MCP_NORMAL);
  }else{ 
    Serial.println("CAN1: Init Fail!");
  }


  //制御データの配列を作成
  txBuf0[0] = (uint8_t)((target_current & 0xff00) >> 8);
  txBuf0[1] = (uint8_t) (target_current & 0x00ff);
  txBuf0[2] = 0;
  txBuf0[3] = 0;
  txBuf0[4] = 0;
  txBuf0[5] = 0;
  txBuf0[6] = 0;
  txBuf0[7] = 0;

  //fdbデータの配列を作成
  txBuf1[0] = (uint8_t)((fdb_angle & 0xff00) >> 8);
  txBuf1[1] = (uint8_t) (fdb_angle & 0x00ff);
  txBuf1[4] = (uint8_t)((fdb_rpm & 0xff00) >> 8);
  txBuf1[5] = (uint8_t) (fdb_rpm & 0x00ff);
  txBuf1[4] = (uint8_t)((fdb_current & 0xff00) >> 8);
  txBuf1[5] = (uint8_t) (fdb_current & 0x00ff);
  txBuf1[6] = (uint8_t) (fdb_temp & 0x00ff);
  txBuf1[7] = 0;

  Pre_millis = millis();
}

void loop(){
  static int count=0; 
  if(millis()-Pre_millis > 100){ // Period: 100ms
    CAN0.sendMsgBuf(0x200, 0, 8, txBuf0);
    CAN1.sendMsgBuf(0x201, 0, 8, txBuf1);
    Serial.println("Send ID: 0x200,0x201");
    Pre_millis=millis();
  }

  if(CAN0.checkReceive()==CAN_MSGAVAIL){
    CAN0.readMsgBuf(&rxId, &len, rxBuf);
    Serial.print("Recive ID: ");
    Serial.print(rxId, HEX);
    Serial.print(" Data: ");
    for(byte i = 0; i<len; i++){
      Serial.print(rxBuf[i], HEX);
      Serial.print(" ");
    }
  }

  if(CAN1.checkReceive()==CAN_MSGAVAIL){
    CAN1.readMsgBuf(&rxId, &len, rxBuf);
    Serial.print("Recive ID: ");
    Serial.print(rxId, HEX);
    Serial.print(" Data: ");
    for(byte i = 0; i<len; i++){
      Serial.print(rxBuf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}