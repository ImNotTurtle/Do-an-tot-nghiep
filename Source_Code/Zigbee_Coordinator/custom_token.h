// Define token names here
#define NVM3KEY_LED1_ON_OFF			(NVM3KEY_DOMAIN_USER | 0x0001)
#define NVM3KEY_DEVICE_MANAGER		(NVM3KEY_DOMAIN_USER | 0x0001)



#if defined(DEFINETYPES)
// Include or define any typedef for tokens here
typedef struct {
  uint8_t ledIndex;     // LED index
  bool ledOnOff;        // LED ON OFF status
} ledOnOffStatus_t;

typedef enum{
	OFFLINE,
	ONLINE,
	UNKNOWN = 0xFF
}ConnectState_e;

typedef enum{
	HC,
	REM,
	QUAT,
	UNQUALIFIED = 0xFF,
}DeviceType_e;

typedef struct{
	DeviceType_e type;
	EmberNodeId nodeId;
	ConnectState_e state;
	uint32_t lastTime;
}Device_s;

typedef struct{
	Device_s deviceList[10];
	uint8_t length;
}DeviceManager_s;
#endif

#ifdef DEFINETOKENS
// Define the actual token storage information here
DEFINE_BASIC_TOKEN(DEVICE_MANAGER,
		DeviceManager_s,
		{{0x00}, 0})
#endif
//#endif
