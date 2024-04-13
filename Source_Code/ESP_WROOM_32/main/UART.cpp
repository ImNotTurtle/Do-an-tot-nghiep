#include "UART.h"

static UART::Queue uart1RxQueue;
static UART::rxHandleCallback UART1_rxHandleCallback;
static UART::UART_FRAME PollRxBuffer(uint8_t port);
static void ProcessSerialReceiver(void);


static UART::UART_FRAME PollRxBuffer(uint8_t port){
  UART::UART_FRAME retFrame = UART_FRAME_INIT_DEFAULT;
  if(port == UART_NUM_1){
      uint8_t byte = 0;
      if(uart1RxQueue.GetFront(&byte) && byte == UART_FRAME_START_OF_FRAME){
        uart1RxQueue.DeQueue(NULL);
        uart1RxQueue.DeQueue(&retFrame.length);
        uint8_t byte1 = 0, byte2 = 0;
        uart1RxQueue.DeQueue(&byte1);
        uart1RxQueue.DeQueue(&byte2);
        retFrame.nodeId = (uint16_t)((byte1 << 8) | byte2);
        uart1RxQueue.DeQueue(&retFrame.endpoint);
        uart1RxQueue.DeQueue(&retFrame.id);
        uart1RxQueue.DeQueue(&retFrame.type);
        uart1RxQueue.DeQueue(&retFrame.payloadLength);
        for(uint8_t i = 0; i < retFrame.payloadLength; i++){
          uart1RxQueue.DeQueue(&retFrame.payload[i]);
        }
        uart1RxQueue.DeQueue(&retFrame.sequence);
        uart1RxQueue.DeQueue(&retFrame.cxor);
        if(UART::UART_FRAME_CheckXor(retFrame)){
          return retFrame;
        }
      }
      else{
        uart1RxQueue.DeQueue(NULL);
      }
  }
}
static void ProcessSerialReceiver(void* arg){
  //ticking every 200ms
  const TickType_t xFrequency = pdMS_TO_TICKS( 200 );
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while(1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    //poll uart1 rx buffer
    UART::UART_FRAME frame = UART_FRAME_INIT_DEFAULT;
    frame = PollRxBuffer(UART_NUM_1);
    if(frame.length > 0){
      UART1_rxHandleCallback(frame);
    }

    xLastWakeTime = xTaskGetTickCount();
  }
  vTaskDelete(NULL);
}

void UART::UART1_Init(rxHandleCallback cb){
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  };
  uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
  uart_param_config(UART_NUM_1, &uart_config);
  uart_set_pin(UART_NUM_1, UART1_TX_PIN, UART1_RX_PIN, UART1_RTS_PIN, UART1_CTS_PIN);

  xTaskCreate(&UART::UART1_rxTask, "UART1_rxTask", RX_BUF_SIZE * 2, NULL, configMAX_PRIORITIES - 1, NULL);
  xTaskCreate(&ProcessSerialReceiver, "ProcessSerialReceiver", RX_BUF_SIZE * 2, NULL, configMAX_PRIORITIES - 2, NULL);

  UART1_rxHandleCallback = cb;
  uart1RxQueue.Init(128);
}


void UART::UART1_rxTask(void *arg){
  uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE + 1);
  while (1) {
    const uint8_t rxBytes = (uint8_t)uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
    //push data to the queue
    if(rxBytes > 0){
      for(uint8_t i = 0; i < rxBytes; i++){
        uart1RxQueue.EnQueue(data[i]);
      }
    }
  }
  free(data);
}



//transmit the frame through UARTx port and return the number of bytes transmitted
uint8_t UART::UART_SendFrame(uart_port_t UARTx, UART::UART_FRAME frame){
  uint8_t txBytes = 0;
  uint8_t SoF = UART_FRAME_START_OF_FRAME;
  txBytes += (uint8_t)uart_write_bytes(UARTx, &SoF, 1);
  txBytes += (uint8_t)uart_write_bytes(UARTx, &frame.length, 1);
  uint8_t byte1 = (frame.nodeId >> 8) & 0xFF, byte2 = frame.nodeId & 0xFF;
  txBytes += (uint8_t)uart_write_bytes(UARTx, &byte1, 1);
  txBytes += (uint8_t)uart_write_bytes(UARTx, &byte2, 1);
  txBytes += (uint8_t)uart_write_bytes(UARTx, &frame.endpoint, 1);
  txBytes += (uint8_t)uart_write_bytes(UARTx, &frame.id, 1);
  txBytes += (uint8_t)uart_write_bytes(UARTx, &frame.type, 1);
  txBytes += (uint8_t)uart_write_bytes(UARTx, &frame.payloadLength, 1);
  for(uint8_t i = 0; i < frame.payloadLength; i++){
    txBytes += (uint8_t)uart_write_bytes(UARTx, &frame.payload[i], 1);
  }
  txBytes += (uint8_t)uart_write_bytes(UARTx, &frame.sequence, 1);
  txBytes += (uint8_t)uart_write_bytes(UARTx, &frame.cxor, 1);
  return txBytes;
}


UART::UART_FRAME UART::UART_FRAME_GenerateFrame(uint16_t nodeId, uint8_t endpoint, uint8_t id, uint8_t type, uint8_t payloadLength, uint8_t* payload){
  static uint8_t sequence = 0;
  UART_FRAME retFrame = UART_FRAME_INIT_DEFAULT;
  retFrame.length = payloadLength + 7;
  retFrame.nodeId = nodeId;
  retFrame.endpoint = endpoint;
  retFrame.id = id;
  retFrame.type= type;
  retFrame.payloadLength = payloadLength;
  if(payload != NULL){
    for(uint8_t i = 0; i < payloadLength; i++){
      retFrame.payload[i] = payload[i];
    }
  }
  retFrame.sequence = sequence++;
  retFrame.cxor = UART_FRAME_CalcCxor(retFrame);
  return retFrame;
}
UART::UART_FRAME UART::UART_FRAME_GenerateLevelControl(uint16_t nodeId, uint8_t endpoint, uint8_t level){
  const int payloadSize = 1;
  uint8_t payload[payloadSize] = {0};
  payload[0] = level;
  return UART_FRAME_GenerateFrame(nodeId, endpoint, UART_FRAME_CMD_ID_LEVEL_CONTROL, UART_FRAME_CMD_TYPE_SET, payloadSize, payload);
}
UART::UART_FRAME UART::UART_FRAME_GenerateNetwokUpdate(void){
  return UART_FRAME_GenerateFrame(0, 0x00, UART_FRAME_CMD_ID_NETWORK_REPORT, UART_FRAME_CMD_TYPE_GET, 0, NULL);
}

UART::UART_FRAME UART::UART_FRAME_GenerateOpenNetwork(void){
  return UART::UART_FRAME_GenerateFrame(0x0000, 0x01, UART_FRAME_CMD_ID_NETWORK_OPENING, UART_FRAME_CMD_TYPE_SET, 0, NULL);
}
UART::UART_FRAME UART::UART_FRAME_GenerateCloseNetwork(void){
  return UART::UART_FRAME_GenerateFrame(0x0000, 0x01, UART_FRAME_CMD_ID_NETWORK_CLOSING, UART_FRAME_CMD_TYPE_SET, 0, NULL);
}
UART::UART_FRAME UART::UART_FRAME_GenerateCreateNetwork(void){
  return UART::UART_FRAME_GenerateFrame(0x0000, 0x01, UART_FRAME_CMD_ID_NETWORK_CREATING, UART_FRAME_CMD_TYPE_SET, 0, NULL);
}

uint8_t UART::UART_FRAME_CalcCxor(UART::UART_FRAME frame){
  uint8_t cxor = UART_FRAME_CHECKXOR_INIT;
  uint8_t byte1 = (frame.nodeId >> 8) & 0xFF, byte2 = frame.nodeId & 0xFF;
  cxor ^= byte1;
  cxor ^= byte2;
  cxor ^= frame.endpoint;
  cxor ^= frame.id;
  cxor ^= frame.type;
  cxor ^= frame.payloadLength;
  for(uint8_t i = 0; i < frame.payloadLength; i++){ 
    cxor ^= frame.payload[i];
  }
  cxor ^= frame.sequence;
  return cxor;
}
bool UART::UART_FRAME_CheckXor(UART::UART_FRAME frame){
  return UART_FRAME_CalcCxor(frame) ==  frame.cxor;
}


std::string UART::ToString(UART_FRAME frame){
  std::string retStr = "";
  retStr += "Frame: ";
  retStr += std::to_string(frame.length) + " ";
  retStr += std::to_string(frame.nodeId) + " ";
  retStr += std::to_string(frame.endpoint) + " ";
  retStr += std::to_string(frame.id) + " ";
  retStr += std::to_string(frame.type) + " ";
  retStr += std::to_string(frame.payloadLength) + " ";
  for(uint8_t i = 0; i < frame.payloadLength; i++){
    retStr += std::to_string(frame.payload[i]) + " ";
  }
  retStr += std::to_string(frame.sequence) + " ";
  retStr += std::to_string(frame.cxor);
  return retStr;
}




/***********************************************************************************
*                                   QUEUE                                          *            
***********************************************************************************/

UART::Queue::Queue(){

}

void UART::Queue::Init(uint8_t bufferSize){
  this->buffer = new uint8_t[bufferSize];
  this->bufferSize = bufferSize;
  this->frontPos = 0;
  this->backPos = 0;
}

bool UART::Queue::IsEmpty(void){
  return this->frontPos == this->backPos;
}

bool UART::Queue::IsFull(void){
  if(this->backPos > this->frontPos) return this->backPos - this->frontPos == this->bufferSize;
  else if(this->backPos < this->frontPos) return this->frontPos - this->backPos == this->bufferSize;
  return false;
}

void UART::Queue::EnQueue(uint8_t data){
  this->buffer[this->backPos++] = data;
  if(this->backPos == this->bufferSize){
    this->backPos = 0;
  }
}

bool UART::Queue::DeQueue(uint8_t *data){
  if(IsEmpty()) return false;
  if(data != NULL){
    *data = this->buffer[this->frontPos];
  }
  this->frontPos++;
  if(this->frontPos == this->bufferSize){
    this->frontPos = 0;
  }
  return true;
}

bool UART::Queue::GetFront(uint8_t *data){
  if(IsEmpty()) return false;
  *data = this->buffer[this->frontPos];
  return true;
}