/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/
/* Fill-in information from Blynk Device Info here */
/*
TODO
  Update device state that current in the network
  Level control from button
*/
#define BLYNK_TEMPLATE_ID "TMPL66zeQ5V79"
#define BLYNK_TEMPLATE_NAME "wifi gateway"
#define BLYNK_AUTH_TOKEN "3JnVdB-LnTjTEcB5Xe1prlB5hub-MOeF"


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "UART.h"
#include <stdbool.h>
#include <vector>
#include <utility>
#include <tuple>

using namespace std;


#define UPDATE_TO_BLYNK_INTERVAL                    1000L
#define BLINK_CONNECTION_INTERVAL                   1500L

#define LED_ON                                      1
#define LED_OFF                                     0
#define NETWORK_OPEN                                1
#define NETWORK_CLOSE                               0
#define DEVICE_CONNECTED                            1
#define DEVICE_DISCONNECTED                         0

#define DEVICE_NOT_FOUND                            0xFF
#define NODE_ID_NOT_FOUND                           0xFFFF

#define VIRTUAL_PIN_CURTAIN_LEVEL                   V0
#define VIRTUAL_PIN_HC_CONNECTION                   V1
#define VIRTUAL_PIN_HC_NETWORK_CREATE               V2
#define VIRTUAL_PIN_HC_NETWORK_STATE                V3
#define VIRTUAL_PIN_CURTAIN_CONNECTION              V4

#define VIRTUAL_PIN_LIST_DEVICE_ID                  0
#define VIRTUAL_PIN_LIST_DEVICE_TYPE                1
#define VIRTUAL_PIN_LIST_VIRTUAL_PIN_COUNT          2
#define VIRTUAL_PIN_LIST_VIRTUAL_PIN_OFFSET         3

#define VIRTUAL_PIN_HC_CONNECTION_INDEX             0
#define VIRTUAL_PIN_HC_NETWORK_CREATE_INDEX         1
#define VIRTUAL_PIN_HC_NETWORK_STATE_INDEX          2

#define VIRTUAL_PIN_CURTAIN_CONNECTION_INDEX        0
#define VIRTUAL_PIN_CURTAIN_LEVEL_INDEX             1



enum DeviceType_e{
  HC,
  REM,
  UNQUALIFIED = 0xFF,
};

enum UpdateCmd{
  None,
  Curtain_level,
  HC_connection,
  HC_network_create,
  HC_network_state,
  Curtain_connection,
};

enum Connection_e{
  OFFLINE,
  ONLINE,
};

typedef uint8_t VirtualPin;
typedef uint16_t NodeId;
typedef tuple<UpdateCmd, NodeId, uint8_t> UpdateCmdTuple;

struct DeviceVirtualPin_s{
  NodeId id;
  Connection_e connection;
  DeviceType_e type;
  uint8_t pinCount;
  vector<VirtualPin> pinList;
};




char ssid[] = "Duyen Pham";
char pass[] = "99999999";
BlynkTimer timer;
vector<DeviceVirtualPin_s> deviceVirtualPinList; // a list contains device information and its allocated virtual pins
vector<UpdateCmdTuple> dataPendingList; // a list contains pending cmd wait for timer to treat
vector<pair<DeviceType_e, vector<vector<VirtualPin>>>> unallocatedVPList = {//a list of unallocated virtual pin on blynk server for a specific device type
  { HC , { {V1, V2, V3} } },
  { REM, { {V4, V0    } } }
};

/*****************************************************************************
*                         DEVICE MANAGING                                    *
*****************************************************************************/

//get a list of unallocated virtual pins for a specific of device type
vector<VirtualPin> GetUnallocatedVirtualPin(DeviceType_e type){
  //search for the type to get the list
  for(uint8_t i = 0; i < unallocatedVPList.size(); i++){
    if(type == unallocatedVPList[i].first){
      //pop out a list in unallocatedVPList[i].second 
      if(unallocatedVPList[i].second.size() > 0){
        vector<VirtualPin> retList = unallocatedVPList[i].second[0];
        unallocatedVPList[i].second.erase(unallocatedVPList[i].second.begin());
        return retList;
      }
      break;
    }
  }
  return {};
}

bool IsDeviceAllocated(NodeId id){
  for(uint8_t i = 0; i < deviceVirtualPinList.size(); i++){
    if(deviceVirtualPinList[i].id == id){
      return true;
    }
  }
  return false;
}

//get the device index in deviceVirtualPinList, return DEVICE_NOT_FOUND if not found
uint8_t GetDeviceIndex(NodeId id){
  for(uint8_t i = 0; i < deviceVirtualPinList.size(); i++){
    if(deviceVirtualPinList[i].id == id){
      return i;
    }
  }
  return DEVICE_NOT_FOUND;
}

//get device type of the device has been allocated
DeviceType_e GetDeviceType(NodeId id){
  for(uint8_t i = 0; i < deviceVirtualPinList.size(); i++){
    if(deviceVirtualPinList[i].id == id){
      return deviceVirtualPinList[i].type;
    }
  }
  return DeviceType_e::UNQUALIFIED;
}

//add device to list and allocate for that device virtual pins available in the list
void AddDeviceToList(NodeId id, DeviceType_e type){
  if(IsDeviceAllocated(id) == false){//device is not in the list
    vector<VirtualPin> unallocVP = GetUnallocatedVirtualPin(type); // unallocated virtual pin list
    DeviceVirtualPin_s deviceVirtualPin;
    string str = "";
    for(auto i : unallocVP){
      str += to_string(i) + ",";
    }
    deviceVirtualPin.id = id;
    deviceVirtualPin.type = type;
    deviceVirtualPin.pinCount = unallocVP.size();
    deviceVirtualPin.pinList = unallocVP;
    deviceVirtualPinList.push_back(deviceVirtualPin);
  }
  else {
    Serial.printf("Allocate device fail with id: %x, type: %d\n", id, type);
  }
}
//delete device from list and recall virtual pins
void RemoveDeviceFromList(NodeId id){
  uint8_t index = GetDeviceIndex(id);
  if(index != DEVICE_NOT_FOUND){
    //recall virtual pin for other devices to allocate
    vector<VirtualPin> pinList = deviceVirtualPinList[index].pinList;
    DeviceType_e type = deviceVirtualPinList[index].type;
    //find where to add virtual pins
    for(uint8_t i = 0; i < unallocatedVPList.size(); i++){
      if(unallocatedVPList[i].first == type){
        //add virtual pins
        unallocatedVPList[i].second.push_back(pinList);
        break;
      }
    }
    //erase from the list
    Serial.printf("Device remove from network with id: %d, type: %d, recall %d virtual pins\n", id, type, pinList.size());
    deviceVirtualPinList.erase(deviceVirtualPinList.begin() + index);
  }
}

void RemoveAllDevicesFromList(void){
  while(deviceVirtualPinList.size() > 0){
    RemoveDeviceFromList(deviceVirtualPinList[0].id);
  }
}
//return nodeid of the device associated with a virtual pin, return NODE_ID_NOT_FOUND if virtual pin not found
NodeId GetNodeIdWithPin(VirtualPin pin){
  for(auto device : deviceVirtualPinList){
    //search through pin list
    for(auto vpin : device.pinList){
      if(vpin == pin){
        return device.id;
      }
    }
  }
  return NODE_ID_NOT_FOUND;
}

/*****************************************************************************
*                     SERIAL COMMUNICATION HANDLING                          *
*****************************************************************************/

//send data to blynk server frequently
void sendDataToBlynk(void){
  while(dataPendingList.size() > 0){
    UpdateCmdTuple p = dataPendingList[0];
    UpdateCmd cmd = get<0>(p);
    switch(cmd){
      case UpdateCmd::None:
        break;
      case UpdateCmd::Curtain_level:
      {
        NodeId id = get<1>(p);
        uint8_t index = GetDeviceIndex(id);
        uint8_t state = get<2>(p);
        if(index != DEVICE_NOT_FOUND){
          uint8_t virtualPin = deviceVirtualPinList[index].pinList[VIRTUAL_PIN_CURTAIN_LEVEL_INDEX];
          Blynk.virtualWrite(virtualPin, state);
        }
        break;
      }
      case UpdateCmd::HC_network_create:
      {
        Blynk.virtualWrite(VIRTUAL_PIN_HC_NETWORK_CREATE, get<2>(p));
        break;
      }
      
      case UpdateCmd::HC_connection:
      case UpdateCmd::Curtain_connection:
      {
        //update connection state to deviceVirtualPinList
        NodeId id = get<1>(p);
        uint8_t index = GetDeviceIndex(id);
        uint8_t connection = get<2>(p);
        if(index != DEVICE_NOT_FOUND){
          deviceVirtualPinList[index].connection = (Connection_e)connection;
        }
        break;
      }
      case UpdateCmd::HC_network_state:
      {
        Blynk.virtualWrite(VIRTUAL_PIN_HC_NETWORK_STATE, get<2>(p));
        break;
      }
      default:
        break;
    }
    dataPendingList.erase(dataPendingList.begin());
  }
}

//handle blink connection on blynk
void blinkDeviceConnection(void){
  static uint8_t state = LED_ON;//led blink state
  //blink devices that currently appear in the deviceVirtualPinList
  Blynk.beginGroup();
  for(uint8_t i = 0; i < deviceVirtualPinList.size(); i++){
    Connection_e connection = deviceVirtualPinList[i].connection;
    vector<VirtualPin> pinList = deviceVirtualPinList[i].pinList;
    if(pinList.size() > 0 && connection == Connection_e::ONLINE){//valid pin list and is online state
      Blynk.virtualWrite(pinList[0], state);
    }
    else if(pinList.size() > 0 && connection == Connection_e::OFFLINE){
      Blynk.virtualWrite(pinList[0], 0);
    }
  }
  Blynk.endGroup();
  //toggle the state
  if(state == LED_ON){
    state = LED_OFF;
  }
  else state = LED_ON;
}
//handle the frame when received from UART 1
void rxTask(UART::UART_FRAME frame){
  if(frame.length == 0) return;
  NodeId nodeId = frame.nodeId;
  Serial.print("Frame received: ");
  Serial.println((UART::ToString(frame)).c_str());
  switch(frame.id){
    case UART_FRAME_CMD_ID_NETWORK_CREATING:
    {
      //toggle off the control when receive response frame
      if(frame.type == UART_FRAME_CMD_TYPE_RES){
        Serial.println("Create network frame received");
        UpdateCmdTuple p = {UpdateCmd::HC_network_create, nodeId, 0};
        dataPendingList.push_back(p);
      }
      break;
    }
    case UART_FRAME_CMD_ID_NETWORK_OPENING:
    {
      if(frame.type == UART_FRAME_CMD_TYPE_RES){
        Serial.println("Opening network frame received");
        UpdateCmdTuple p = {UpdateCmd::HC_network_state, nodeId, NETWORK_OPEN};
        dataPendingList.push_back(p);
      }
      break;
    }
    case UART_FRAME_CMD_ID_NETWORK_CLOSING:
    {
      if(frame.type == UART_FRAME_CMD_TYPE_RES){
        Serial.println("Closing network frame received");
        UpdateCmdTuple p = {UpdateCmd::HC_network_state, nodeId, NETWORK_CLOSE};
        dataPendingList.push_back(p);
      }
      break;
    }
    case UART_FRAME_CMD_ID_NETWORK_REPORT:
    //syncing device list with coordinator
    {
      if(frame.type == UART_FRAME_CMD_TYPE_SET){
        //erase the list to sync with coordinator
          RemoveAllDevicesFromList();
      }
      else if(frame.type == UART_FRAME_CMD_TYPE_RES){
        //add device to the list, allocate virtual pin for that devide based on its type
        DeviceType_e type = (DeviceType_e)frame.payload[0];
        Connection_e connection = (Connection_e)frame.payload[1];
        AddDeviceToList(nodeId, type);
        uint8_t index = GetDeviceIndex(nodeId);
        string str = "";
        if(index != DEVICE_NOT_FOUND){
          auto pinList = deviceVirtualPinList[index].pinList;
          for(auto i : pinList){
            str += to_string(i) + ", ";
          }
        }
        Serial.printf("Network report device with id: %x, type: %d, pinList: %s\n", nodeId, type, str.c_str());
        //set device connection state
        UpdateCmdTuple p = {UpdateCmd::None, nodeId, connection};
        switch(type){
          case HC:
          {
            get<0>(p) = UpdateCmd::HC_connection;
            break;
          }
          case REM:
          {
            get<0>(p) = UpdateCmd::Curtain_connection;
            break;
          }
          default:
            break;
        }
        dataPendingList.push_back(p);
      }
      break;
    }
    case UART_FRAME_CMD_ID_DEVICE_JOIN_NETWORK:
    {
      if(frame.type == UART_FRAME_CMD_TYPE_RES){
        //add devide to the list, allocate virtual pin for that device based on its type
        DeviceType_e type = (DeviceType_e)frame.payload[0];
        AddDeviceToList(nodeId, type);
        Serial.printf("NodeId: %x\n", nodeId);
        uint8_t index = GetDeviceIndex(nodeId);
        string str = "";
        if(index != DEVICE_NOT_FOUND){
          auto pinList = deviceVirtualPinList[index].pinList;
          for(auto i : pinList){
            str += to_string(i) + ", ";
          }
        }
        Serial.printf("Device join network with id: %x, type: %d\n", nodeId, type);
        //set device connection state
        UpdateCmdTuple p = {UpdateCmd::None, nodeId, Connection_e::ONLINE};
        switch(type){
          case HC:
          {
            get<0>(p) = UpdateCmd::HC_connection;
            break;
          }
          case REM:
          {
            get<0>(p) = UpdateCmd::Curtain_connection;
            break;
          }
          default:
            break;
        }
        dataPendingList.push_back(p);
      }
      break;
    }
    case UART_FRAME_CMD_ID_DEVICE_LEAVE_NETWORK:
    {
      Serial.printf("Device leave network with id: %x\n", nodeId);
      //erase from the list, recall virtual pin
      RemoveDeviceFromList(nodeId);
      break;
    }
    case UART_FRAME_CMD_ID_DEVICE_CONNECTED:
    {
      if(frame.type == UART_FRAME_CMD_TYPE_RES){
        //add the device to the list of device to allocate virtual pins
        DeviceType_e type = (DeviceType_e)frame.payload[0];
        Serial.printf("Device connected with id: %x\n", nodeId);
        AddDeviceToList(nodeId, type);
        //add command to the queue wait for timer to update on server
        UpdateCmdTuple p = {UpdateCmd::None, nodeId, Connection_e::ONLINE};
        switch(type){
          case HC:
          {
            get<0>(p) = UpdateCmd::HC_connection;
            break;
          }
          case REM:
          {
            get<0>(p) = UpdateCmd::Curtain_connection;
            break;
          }
          default:
            break;
        }
        dataPendingList.push_back(p);
        }
        break;
    }
    case UART_FRAME_CMD_ID_DEVICE_DISCONNECTED:
    {
      if(frame.type == UART_FRAME_CMD_TYPE_RES){

        //add command to the queue wait for timer to update on server
        Serial.printf("Device disconnected with id: %x\n", nodeId);
        UpdateCmdTuple p = {UpdateCmd::None, nodeId, Connection_e::OFFLINE};
        DeviceType_e type = GetDeviceType(nodeId);
        switch(type){
          case HC:
          {
            get<0>(p) = UpdateCmd::HC_connection;
            break;
          }
          case REM:
          {
            get<0>(p) = UpdateCmd::Curtain_connection;
            break;
          }
          default:
            break;
        }
        dataPendingList.push_back(p);
      }
      break;
    }
    case UART_FRAME_CMD_ID_LEVEL_CONTROL:
    {
      if(frame.type == UART_FRAME_CMD_TYPE_RES){
        uint8_t level = frame.payload[0];
        Serial.printf("Level control received from id: %x, level: %d\n", nodeId, level);
        UpdateCmdTuple p = {UpdateCmd::Curtain_level, nodeId, level};
        dataPendingList.push_back(p);
      }
      break;
    }
    default:
      break;
  }
}

void setup()
{
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Blynk server connected");
  
  Serial.println("Setup success");

  timer.setInterval(UPDATE_TO_BLYNK_INTERVAL, sendDataToBlynk);
  timer.setInterval(BLINK_CONNECTION_INTERVAL, blinkDeviceConnection);

  UART::UART1_Init(rxTask);

  //send network report frame to coordinator
  UART::UART_FRAME frame = UART::UART_FRAME_GenerateNetwokUpdate();
  Serial.printf("Network report frame sent. %s\n", UART::ToString(frame).c_str());
  UART::UART_SendFrame(UART_NUM_1, frame); 
}

void loop()
{
  Blynk.run();
  timer.run();
}



/*******************************************************************************************
*                                       BLYNK THINGS                                       *
*******************************************************************************************/
BLYNK_WRITE(VIRTUAL_PIN_CURTAIN_LEVEL)
{
  uint8_t value = (uint8_t)param.asInt();
  //search for the device with this virtual pin to send command directly to that device
  NodeId nodeId = GetNodeIdWithPin(VIRTUAL_PIN_CURTAIN_LEVEL);
  if(nodeId != NODE_ID_NOT_FOUND){
    UART::UART_FRAME frame = UART::UART_FRAME_GenerateLevelControl(nodeId, 1, value);
    Serial.println((UART::ToString(frame)).c_str());
    UART::UART_SendFrame(UART_NUM_1, frame);
  }
  
}

BLYNK_WRITE(VIRTUAL_PIN_HC_NETWORK_CREATE)
{
  uint8_t value = (uint8_t)param.asInt();
  NodeId nodeId = GetNodeIdWithPin(VIRTUAL_PIN_HC_NETWORK_CREATE);
  if(nodeId != NODE_ID_NOT_FOUND){
    UART::UART_FRAME frame = UART::UART_FRAME_GenerateCreateNetwork();
    Serial.printf("Network create frame: %s\n", UART::ToString(frame).c_str());
    UART::UART_SendFrame(UART_NUM_1, frame);
  }
}

BLYNK_WRITE(VIRTUAL_PIN_HC_NETWORK_STATE)
{
  uint8_t value = (uint8_t)param.asInt();
  NodeId nodeId = GetNodeIdWithPin(VIRTUAL_PIN_HC_NETWORK_STATE);
  if(nodeId != NODE_ID_NOT_FOUND){
    UART::UART_FRAME frame = UART_FRAME_INIT_DEFAULT;
    if(value == NETWORK_OPEN){  
      frame = UART::UART_FRAME_GenerateOpenNetwork();
    }
    else if(value == NETWORK_CLOSE){
      frame = UART::UART_FRAME_GenerateCloseNetwork();
    }
    Serial.printf("Network state frame: %s\n", UART::ToString(frame).c_str());
    UART::UART_SendFrame(UART_NUM_1, frame);
  }
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{

}

