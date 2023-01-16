#ifndef __C620_H__
#define __C620_H__

  #define C620_ENCORDER_RESOLUTION 8192
  #define DEGREE 360.0f

  void C620_Init(void);
  void C620_Update(void);
  void C620_TransmitCAN(void);
  void C620_ReceiveCAN(void);
  void C620_ConvertReceiveData(void);

#endif  // __C620_H__