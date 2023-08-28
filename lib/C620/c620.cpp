#include <Arduino.h>

/*
  CANモジュールのINT→ 使用しない
  CANモジュールのSCK→ デジタルピン13 (SCLK)
  CANモジュールのSI → デジタルピン11 (MOSI)
  CANモジュールのSO → デジタルピン12 (MISO)
  CANモジュールのCS → デジタルピンD10 (SS) もしくはお好きなデジタルピン
*/

#include <mcp_can.h> //https://github.com/coryjfowler/MCP_CAN_lib
#include <SPI.h>

#include "c620.h"

int16_t target_current;

uint16_t tim_cnt = 0;
uint8_t txBuf0[8];

unsigned long rxId;
byte len;
uint8_t rxBuf[8];


MCP_CAN CAN0(10);// CAN0 CS: pin 10

void C620_Init(){
  // init CAN0 bus, baudrate: 1000kbps@8MHz
  if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK){
    Serial.println("CAN0: Init OK!");
    CAN0.setMode(MCP_NORMAL);
  } 
  else{ 
    Serial.println("CAN0: Init Fail!");
    Serial.println("Please Push Reset Button");
    while(1){}
  }
}

void C620_Update(){
  tim_cnt++;

  if     (tim_cnt <  50 ){ target_current = 1000; } 
  else if(tim_cnt < 100 ){ target_current = 2000; } 
  else                   { tim_cnt = 0 ; }
}

void C620_TransmitCAN(){
  //制御データの配列を作成
  txBuf0[0] = (uint8_t)((target_current & 0xff00) >> 8);
  txBuf0[1] = (uint8_t) (target_current & 0x00ff);
  txBuf0[2] = 0;
  txBuf0[3] = 0;
  txBuf0[4] = 0;
  txBuf0[5] = 0;
  txBuf0[6] = 0;
  txBuf0[7] = 0;

  // canの送信
  CAN0.sendMsgBuf(0x200, 0, 8, txBuf0);
}

void C620_ReceiveCAN(){
  if(CAN0.checkReceive()==CAN_MSGAVAIL){
    CAN0.readMsgBuf(&rxId, &len, rxBuf);
  }
}

void C620_ConvertReceiveData(){
  int16_t fdb_angle_enc = (rxBuf[0] << 8) + rxBuf[1];
  int16_t fdb_angle_deg = DEGREE * fdb_angle_enc / C620_ENCORDER_RESOLUTION;
  int16_t fdb_rpm = (rxBuf[2] << 8) + rxBuf[3];
  int16_t fdb_current = (rxBuf[4] << 8) + rxBuf[5];
  int16_t fdb_temp = rxBuf[6];
  
  Serial.print("target current: "); Serial.print(target_current);
  Serial.print("\tRecive ID: "); Serial.print(rxId, HEX);
  Serial.print("\tangle: "); Serial.print(fdb_angle_deg); Serial.print("[°]");
  Serial.print("\trpm: "); Serial.print(fdb_rpm); Serial.print("[rpm]");
  Serial.print("\tcurrent: "); Serial.print(fdb_current); Serial.print("");
  Serial.print("\ttemp: "); Serial.print(fdb_temp); Serial.print("[℃]");

  Serial.println();
}