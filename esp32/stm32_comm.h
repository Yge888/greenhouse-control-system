// stm32_comm.h
#ifndef STM32_COMM_H
#define STM32_COMM_H

void stm32Init();
void stm32SendCommand(const String& cmd);
void stm32ProcessRecv();

#endif
