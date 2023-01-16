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

#define C620_ENCORDER_RESOLUTION 8192
#define DEGREE 360.0f

unsigned long rxId;
byte len;
uint8_t rxBuf[8];
uint8_t txBuf0[8];
long Pre_millis;

MCP_CAN CAN0(10);// CAN0 CS: pin 10

int16_t target_current = 500;

void setup()
{
  Serial.begin(115200);

  // init CAN0 bus, baudrate: 250kbps@8MHz
  if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK){
    Serial.println("CAN0: Init OK!");
    CAN0.setMode(MCP_NORMAL);
  } else{ 
    Serial.println("CAN0: Init Fail!");
    while(1){

    }
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

  Pre_millis = millis();
}

void loop(){
  static int count=0; 
  if(millis() - Pre_millis > 100){ // Period: 100ms
    CAN0.sendMsgBuf(0x200, 0, 8, txBuf0);
    Serial.println("Send ID: 0x200");
    Pre_millis=millis();
  }

  if(CAN0.checkReceive()==CAN_MSGAVAIL){
    CAN0.readMsgBuf(&rxId, &len, rxBuf);
    Serial.print("Recive ID: ");
    Serial.print(rxId, HEX);

    int16_t fdb_angle_enc = (rxBuf[0] << 8) + rxBuf[1];
    int16_t fdb_angle_deg = DEGREE * fdb_angle_enc / C620_ENCORDER_RESOLUTION;
    int16_t fdb_rpm = (rxBuf[2] << 8) + rxBuf[3];
    int16_t fdb_current = (rxBuf[4] << 8) + rxBuf[5];
    int16_t fdb_temp = rxBuf[6];

    Serial.print("\t\tangle: "); Serial.print(fdb_angle_deg); Serial.print("[°]");
    Serial.print("\trpm: "); Serial.print(fdb_rpm); Serial.print("[rpm]");
    Serial.print("\tcurrent: "); Serial.print(fdb_current); Serial.print("");
    Serial.print("\ttemp: "); Serial.print(fdb_temp); Serial.println("[℃]");
  }
  delay(100);
}