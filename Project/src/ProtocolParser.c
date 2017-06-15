#include "ProtocolParser.h"
#include "_global.h"
#include "MyMessage.h"
#include "xliNodeConfig.h"

uint8_t bMsgReady = 0;

// Assemble message
void build(uint8_t _destination, uint8_t _sensor, uint8_t _command, uint8_t _type, bool _enableAck, bool _isAck)
{
    msg.header.version_length = PROTOCOL_VERSION;
    msg.header.sender = gConfig.nodeID;
    msg.header.destination = _destination;
    msg.header.sensor = _sensor;
    msg.header.type = _type;
    miSetCommand(_command);
    miSetRequestAck(_enableAck);
    miSetAck(_isAck);
}

uint8_t ParseProtocol(){
  if( msg.header.destination != gConfig.nodeID ) return 0;
  
  uint8_t _cmd = miGetCommand();
  uint8_t _sender = msg.header.sender;  // The original sender
  uint8_t _type = msg.header.type;
  uint8_t _sensor = msg.header.sensor;
  bool _needAck = (bool)miGetRequestAck();
  bool _isAck = (bool)miGetAck();
  
  switch( _cmd ) {
  case C_INTERNAL:
    if( _type == I_ID_RESPONSE ) {
      // Device/client got nodeID from Controller
      uint8_t lv_nodeID = _sensor;
      if( lv_nodeID == NODEID_GATEWAY || lv_nodeID == NODEID_DUMMY ) {
      } else {
        if( miGetLength() > 8 ) {
          // Verify _uniqueID        
          if(!isIdentityEqual(_uniqueID, msg.payload.data+8, UNIQUE_ID_LEN)) {
            return 0;
          }
        }
        gConfig.nodeID = lv_nodeID;
        memcpy(gConfig.NetworkID, msg.payload.data, sizeof(gConfig.NetworkID));
        gIsChanged = TRUE;
        GotNodeID();
      }
    } else if( _type == I_REBOOT ) {
      if( IS_MINE_SUBID(_sensor) ) {
        // Verify token
        //if(!gConfig.present || gConfig.token == msg.payload.uiValue) {
          // Soft reset
          WWDG->CR = 0x80;
        //}
        return 0;
      }
    } else if( _type == I_CONFIG ) {
      // Node Config
      switch( _sensor ) {
      case NCF_QUERY:
        // Inform controller with version & NCF data
        Msg_NodeConfigData(_sender);
        return 1;
        break;

      case NCF_DEV_SET_SUBID:
        gConfig.subID = msg.payload.data[0];
        break;

      case NCF_DEV_MAX_NMRT:
        gConfig.rptTimes = msg.payload.data[0];
        break;
        
      case NCF_MAP_SENSOR:
        gConfig.senMap = msg.payload.data[0] + msg.payload.data[1] * 256;
        break;
      }
      gIsChanged = TRUE;
      Msg_NodeConfigAck(_sender, _sensor);
      return 1;
    }
    break;
    
  case C_PRESENTATION:
    if( _sensor == S_ZENSENSOR ) {
      if( _isAck ) {
        // Device/client got Response to Presentation message, ready to work
        gConfig.token = msg.payload.uiValue;
        gConfig.present = (gConfig.token >  0);
        GotPresented();
        gIsChanged = TRUE;
      }
    }
    break;
    
  case C_REQ:
    if( _needAck ) {
      // ToDo:
    }    
    break;
    
  case C_SET:
    break;
  }
  
  return 0;
}

void Msg_NodeConfigAck(uint8_t _to, uint8_t _ncf) {
  build(_to, _ncf, C_INTERNAL, I_CONFIG, 0, 1);

  msg.payload.data[0] = 1;      // OK
  miSetPayloadType(P_BYTE);
  miSetLength(1);
  bMsgReady = 1;
}

// Prepare NCF query ack message
void Msg_NodeConfigData(uint8_t _to) {
  uint8_t payl_len = 0;
  build(_to, NCF_QUERY, C_INTERNAL, I_CONFIG, 0, 1);

  msg.payload.data[payl_len++] = gConfig.version;
  msg.payload.data[payl_len++] = gConfig.subID;
  msg.payload.data[payl_len++] = gConfig.type;
  msg.payload.data[payl_len++] = gConfig.senMap % 256;
  msg.payload.data[payl_len++] = gConfig.senMap / 256;
  msg.payload.data[payl_len++] = gConfig.rptTimes;
  msg.payload.data[payl_len++] = 0;     // Reservered
  msg.payload.data[payl_len++] = 0;     // Reservered
  msg.payload.data[payl_len++] = 0;     // Reservered
  msg.payload.data[payl_len++] = 0;     // Reservered
  msg.payload.data[payl_len++] = 0;     // Reservered
  msg.payload.data[payl_len++] = 0;     // Reservered
  
  miSetLength(payl_len);
  miSetPayloadType(P_CUSTOM);
  bMsgReady = 1;
}

void Msg_RequestNodeID() {
  // Request NodeID for device
  build(BASESERVICE_ADDRESS, NODE_TYP_SYSTEM, C_INTERNAL, I_ID_REQUEST, 1, 0);
  miSetPayloadType(P_ULONG32);
  miSetLength(UNIQUE_ID_LEN);
  memcpy(msg.payload.data, _uniqueID, UNIQUE_ID_LEN);
  bMsgReady = 1;
}

// Prepare device presentation message
void Msg_Presentation() {
  build(NODEID_GATEWAY, S_ZENSENSOR, C_PRESENTATION, gConfig.type, 1, 0); // S_LIGHT, S_ZENSENSOR
  miSetPayloadType(P_ULONG32);
  miSetLength(UNIQUE_ID_LEN);
  memcpy(msg.payload.data, _uniqueID, UNIQUE_ID_LEN);
  bMsgReady = 1;
}

#ifdef EN_SENSOR_ALS
// Prepare ALS message
void Msg_SenALS(uint8_t _value) {
  build(NODEID_GATEWAY, S_LIGHT_LEVEL, C_PRESENTATION, V_LIGHT_LEVEL, 0, 0);
  miSetPayloadType(P_BYTE);
  miSetLength(1);
  msg.payload.data[0] = _value;
  bMsgReady = 1;
}
#endif

#ifdef EN_SENSOR_PIR
// Prepare PIR message
void Msg_SenPIR(bool _sw) {
  build(NODEID_GATEWAY, S_IR, C_PRESENTATION, V_STATUS, 0, 0);
  miSetPayloadType(P_BYTE);
  miSetLength(1);
  msg.payload.data[0] = _sw;
  bMsgReady = 1;
}
#endif
  
#ifdef EN_SENSOR_PM25
// Prepare PM2.5 message
void Msg_SenPM25(uint16_t _value) {
  build(NODEID_GATEWAY, S_DUST, C_PRESENTATION, V_LEVEL, 0, 0);
  miSetPayloadType(P_UINT16);
  miSetLength(2);
  msg.payload.data[0] = _value % 256;
  msg.payload.data[1] = _value / 256;
  bMsgReady = 1;  
}
#endif