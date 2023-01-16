#include <Arduino.h>

#include "interrupt.h"

#include "c620.h"

void Tim_Interrupt(){

  C620_Update();
  C620_ConvertReceiveData();
}