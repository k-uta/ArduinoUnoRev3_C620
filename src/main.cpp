#include <Arduino.h>

#include <MsTimer2.h>

#include "interrupt.h"
#include "c620.h"

#define PERIOD 10 // [ms]

void setup()
{
  Serial.begin(115200);
  C620_Init();

  // タイマー割込み開始
  MsTimer2::set(PERIOD, Tim_Interrupt);
  MsTimer2::start();
}

void loop(){

  C620_TransmitCAN();
  C620_ReceiveCAN();
  
}