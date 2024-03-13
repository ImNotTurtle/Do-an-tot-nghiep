#ifndef UART_H
#define UART_H

#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
// #include "Queue.h"

#include <string>


#define UART1_TX_PIN                                          4
#define UART1_RX_PIN                                          22
#define UART1_CTS_PIN                                         UART_PIN_NO_CHANGE
#define UART1_RTS_PIN                                         UART_PIN_NO_CHANGE
#define UART1_QUEUE_BUFFER_SIZE                               128
#define RX_BUF_SIZE                                           1024


#define UART_FRAME_CMD_ID_NETWORK_CREATING                    0x01
#define UART_FRAME_CMD_ID_NETWORK_OPENING                     0x02
#define UART_FRAME_CMD_ID_NETWORK_CLOSING                     0x03
#define UART_FRAME_CMD_ID_NETWORK_REPORT                      0x04
#define UART_FRAME_CMD_ID_DEVICE_JOIN_NETWORK                 0x05
#define UART_FRAME_CMD_ID_DEVICE_LEAVE_NETWORK                0x06
#define UART_FRAME_CMD_ID_DEVICE_CONNECTED                    0x07
#define UART_FRAME_CMD_ID_DEVICE_DISCONNECTED                 0x08
#define UART_FRAME_CMD_ID_LEVEL_CONTROL			                  0x09

#define UART_FRAME_CMD_TYPE_SET                               0
#define UART_FRAME_CMD_TYPE_GET                               1
#define UART_FRAME_CMD_TYPE_RES                               2

#define UART_FRAME_START_OF_FRAME                             ((uint8_t)0xB1)
#define UART_FRAME_CHECKXOR_INIT                              ((uint8_t)0xFF)
#define UART_FRAME_INIT_DEFAULT                               {0, 0, 0, 0, 0, 0, {0}, 0, UART_FRAME_CHECKXOR_INIT}

#define UART_FRAME_PAYLOAD_MAX_SIZE                           4


namespace UART {
  struct UART_FRAME{
    uint8_t length;
    uint16_t nodeId;
    uint8_t endpoint;
    uint8_t id;
    uint8_t type;
    uint8_t payloadLength;
    uint8_t payload[UART_FRAME_PAYLOAD_MAX_SIZE];
    uint8_t sequence;
    uint8_t cxor;
  };

  class Queue{
    public:
      uint8_t* buffer;
      uint8_t bufferSize;
      uint8_t frontPos;
      uint8_t backPos;

      Queue();
      void Init(uint8_t bufferSize);
      bool IsEmpty(void);
      bool IsFull(void);
      void EnQueue(uint8_t data);
      bool DeQueue(uint8_t *data);
      bool GetFront(uint8_t *data);
  };

  typedef void (*rxHandleCallback)(UART::UART_FRAME);


  //functions
  void UART1_Init(rxHandleCallback cb);
  void UART1_rxTask(void *arg);

  UART_FRAME UART_FRAME_GenerateFrame(uint16_t nodeId, uint8_t endpoint, uint8_t id, uint8_t type, uint8_t payloadLength, uint8_t payload[]);
  UART_FRAME UART_FRAME_GenerateLevelControl(uint16_t nodeId, uint8_t endpoint, uint8_t level);
  UART_FRAME UART_FRAME_GenerateNetwokUpdate(void);
  UART_FRAME UART_FRAME_GenerateOpenNetwork(void);
  UART_FRAME UART_FRAME_GenerateCloseNetwork(void);
  UART_FRAME UART_FRAME_GenerateCreateNetwork(void);

  uint8_t UART_SendFrame(uart_port_t UARTx, UART_FRAME frame);
  uint8_t UART_FRAME_GetFrameSize(const UART_FRAME& frame);
  uint8_t UART_FRAME_CalcCxor(UART_FRAME frame);
  bool UART_FRAME_CheckXor(UART_FRAME frame);

  std::string ToString(UART_FRAME frame);
}


#endif