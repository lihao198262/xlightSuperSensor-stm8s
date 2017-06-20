#include "ProtocolParser.h"
#include "_global.h"
#include "MyMessage.h"
#include "relay_key.h"
#include "xliNodeConfig.h"
#include "infrared.h"

uint8_t bMsgReady = 0;

// Assemble message
void build(uint8_t _destination, uint8_t _sensor, uint8_t _command, uint8_t _type, bool _enableAck, bool _isAck)
{
    sndMsg.header.version_length = PROTOCOL_VERSION;
    sndMsg.header.sender = gConfig.nodeID;
    sndMsg.header.destination = _destination;
    sndMsg.header.sensor = _sensor;
    sndMsg.header.type = _type;
    moSetCommand(_command);
    moSetRequestAck(_enableAck);
    moSetAck(_isAck);
}

uint8_t ParseProtocol(){
  if( rcvMsg.header.destination != gConfig.nodeID ) return 0;
  
  uint8_t _cmd = miGetCommand();
  uint8_t _sender = rcvMsg.header.sender;  // The original sender
  uint8_t _type = rcvMsg.header.type;
  uint8_t _sensor = rcvMsg.header.sensor;
  bool _needAck = (bool)miGetRequestAck();
  bool _isAck = (bool)miGetAck();
  bool _OnOff;
  
  switch( _cmd ) {
  case C_INTERNAL:
    if( _type == I_ID_RESPONSE ) {
      // Device/client got nodeID from Controller
      uint8_t lv_nodeID = _sensor;
      if( lv_nodeID == NODEID_GATEWAY || lv_nodeID == NODEID_DUMMY ) {
      } else {
        if( miGetLength() > 8 ) {
          // Verify _uniqueID        
          if(!isIdentityEqual(_uniqueID, rcvMsg.payload.data+8, UNIQUE_ID_LEN)) {
            return 0;
          }
        }
        gConfig.nodeID = lv_nodeID;
        memcpy(gConfig.NetworkID, rcvMsg.payload.data, sizeof(gConfig.NetworkID));
        gIsChanged = TRUE;
        GotNodeID();
      }
    } else if( _type == I_REBOOT ) {
      if( IS_MINE_SUBID(_sensor) ) {
        // Verify token
        //if(!gConfig.present || gConfig.token == rcvMsg.payload.uiValue) {
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
        gConfig.subID = rcvMsg.payload.data[0];
        break;

      case NCF_DEV_MAX_NMRT:
        gConfig.rptTimes = rcvMsg.payload.data[0];
        break;
        
      case NCF_MAP_SENSOR:
        gConfig.senMap = rcvMsg.payload.data[0] + rcvMsg.payload.data[1] * 256;
        break;
      }
      gIsChanged = TRUE;
      Msg_NodeConfigAck(_sender, _sensor);
      return 1;
    }
    break;
    
  case C_PRESENTATION:
#ifdef ZENSENSOR    
    if( _sensor == S_ZENSENSOR ) {
#else      
    if( _sensor == S_ZENREMOTE ) {
#endif      
      if( _isAck ) {
        // Device/client got Response to Presentation message, ready to work
        gConfig.token = rcvMsg.payload.uiValue;
        gConfig.present = (gConfig.token >  0);
        GotPresented();
        gIsChanged = TRUE;
      }
    }
    break;
    
  case C_REQ:
    if( _needAck ) {
      if( IS_MINE_SUBID(_sensor) ) {
        if( _type == V_STATUS ) {
          Msg_DevOnOff(_sender);
          return 1;
        } else if( _type == V_RELAY_ON || _type == V_RELAY_OFF ) {
          _OnOff = relay_get_key(rcvMsg.payload.data[0]);
          Msg_Relay_Ack(_sender, _OnOff ? V_RELAY_ON : V_RELAY_OFF, rcvMsg.payload.data[0]);
          return 1;
        }
      }
    }    
    break;
    
  case C_SET:
    if( IS_MINE_SUBID(_sensor) && !_isAck ) {
      if( _type == V_STATUS && gConfig.nodeID == NODEID_SUPERSENSOR ) {
        // set zensensor on/off
        _OnOff = (rcvMsg.payload.bValue == DEVICE_SW_TOGGLE ? gConfig.state == DEVICE_SW_OFF : rcvMsg.payload.bValue == DEVICE_SW_ON);
        gConfig.state = _OnOff;
        gIsChanged = TRUE;
        if( _needAck ) {
          Msg_DevOnOff(_sender);
          return 1;
        }
      } else if( _type == V_RELAY_ON || _type == V_RELAY_OFF ) {
        for( uint8_t idx = 0; idx < miGetLength(); idx++ ) {
          _OnOff = relay_set_key(rcvMsg.payload.data[idx], _type == V_RELAY_ON);
          if( _needAck ) {
            Msg_Relay_Ack(_sender, _OnOff ? V_RELAY_ON : V_RELAY_OFF, rcvMsg.payload.data[idx]);
            SendMyMessage();
          }
        }
      } else {
        // Parsing payload
        unsigned long buf[2];
        switch(rcvMsg.payload.data[1]) {
        case '1':
          buf[0] = 0x00FF00FFL;
          break;
        case '2':
          buf[0] = 0x00FF8877L;
          break;
        case '3':
          buf[0] = 0x00FFC837L;
          break;
        case '4':
          buf[0] = 0x00FF08F7L;
          break;
        case '5':
          buf[0] = 0x00FF28D7L;
          break;
        case '6':
          buf[0] = 0x00FF48B7L;
          break;
        case '7':
          buf[0] = 0x00FF54ABL;
          break;
        case '8':
          buf[0] = 0x00FF708FL;
          break;
        case '9':
          buf[0] = 0x00FF946BL;
          break;
        default:
          buf[0] = 0xFFFFFFFFL;
          break;
        }
        Set_Send_Buf(buf, 1);
      }
    }
    break;
  }
  
  return 0;
}

void Msg_NodeConfigAck(uint8_t _to, uint8_t _ncf) {
  build(_to, _ncf, C_INTERNAL, I_CONFIG, 0, 1);

  sndMsg.payload.data[0] = 1;      // OK
  moSetPayloadType(P_BYTE);
  moSetLength(1);
  bMsgReady = 1;
}

// Prepare NCF query ack message
void Msg_NodeConfigData(uint8_t _to) {
  uint8_t payl_len = 0;
  build(_to, NCF_QUERY, C_INTERNAL, I_CONFIG, 0, 1);

  sndMsg.payload.data[payl_len++] = gConfig.version;
  sndMsg.payload.data[payl_len++] = gConfig.subID;
  sndMsg.payload.data[payl_len++] = gConfig.type;
  sndMsg.payload.data[payl_len++] = gConfig.senMap % 256;
  sndMsg.payload.data[payl_len++] = gConfig.senMap / 256;
  sndMsg.payload.data[payl_len++] = gConfig.rptTimes;
  sndMsg.payload.data[payl_len++] = 0;     // Reservered
  sndMsg.payload.data[payl_len++] = 0;     // Reservered
  sndMsg.payload.data[payl_len++] = 0;     // Reservered
  sndMsg.payload.data[payl_len++] = 0;     // Reservered
  sndMsg.payload.data[payl_len++] = 0;     // Reservered
  sndMsg.payload.data[payl_len++] = 0;     // Reservered
  
  moSetLength(payl_len);
  moSetPayloadType(P_CUSTOM);
  bMsgReady = 1;
}

void Msg_RequestNodeID() {
  // Request NodeID for device
  build(BASESERVICE_ADDRESS, NODE_TYP_SYSTEM, C_INTERNAL, I_ID_REQUEST, 1, 0);
  moSetPayloadType(P_ULONG32);
  moSetLength(UNIQUE_ID_LEN);
  memcpy(sndMsg.payload.data, _uniqueID, UNIQUE_ID_LEN);
  bMsgReady = 1;
}

// Prepare device presentation message
void Msg_Presentation() {
#ifdef ZENSENSOR
  build(NODEID_GATEWAY, S_ZENSENSOR, C_PRESENTATION, gConfig.type, 1, 0);
#else  
  build(NODEID_GATEWAY, S_ZENREMOTE, C_PRESENTATION, gConfig.type, 1, 0);
#endif

  moSetPayloadType(P_ULONG32);
  moSetLength(UNIQUE_ID_LEN);
  memcpy(sndMsg.payload.data, _uniqueID, UNIQUE_ID_LEN);
  bMsgReady = 1;
}

// Prepare device On/Off status message
void Msg_DevOnOff(uint8_t _to) {
  build(_to, gConfig.subID, C_REQ, V_STATUS, 0, 1);
  moSetLength(1);
  moSetPayloadType(P_BYTE);
  sndMsg.payload.bValue = gConfig.state;
  bMsgReady = 1;
}

// Prepare relay key ACK message
void Msg_Relay_Ack(uint8_t _to, uint8_t _type, uint8_t _key) {
  build(_to, gConfig.subID, C_REQ, _type, 0, 1);
  moSetLength(1);
  moSetPayloadType(P_BYTE);
  sndMsg.payload.bValue = _key;
  bMsgReady = 1;
}

#ifdef EN_SENSOR_ALS
// Prepare ALS message
void Msg_SenALS(uint8_t _value) {
  build(NODEID_GATEWAY, S_LIGHT_LEVEL, C_PRESENTATION, V_LIGHT_LEVEL, 0, 0);
  moSetPayloadType(P_BYTE);
  moSetLength(1);
  sndMsg.payload.data[0] = _value;
  bMsgReady = 1;
}
#endif

#ifdef EN_SENSOR_PIR
// Prepare PIR message
void Msg_SenPIR(bool _sw) {
  build(NODEID_GATEWAY, S_IR, C_PRESENTATION, V_STATUS, 0, 0);
  moSetPayloadType(P_BYTE);
  moSetLength(1);
  sndMsg.payload.data[0] = _sw;
  bMsgReady = 1;
}
#endif
  
#ifdef EN_SENSOR_PM25
// Prepare PM2.5 message
void Msg_SenPM25(uint16_t _value) {
  build(NODEID_GATEWAY, S_DUST, C_PRESENTATION, V_LEVEL, 0, 0);
  moSetPayloadType(P_UINT16);
  moSetLength(2);
  sndMsg.payload.data[0] = _value % 256;
  sndMsg.payload.data[1] = _value / 256;
  bMsgReady = 1;  
}
#endif

#ifdef EN_SENSOR_DHT
// Prepare DHT message
/*
type 0  ---- all
type 1  ---- tem
type 2  ---- hum
*/
void Msg_SenDHT(uint16_t dht_t,uint16_t dht_h,u8 type) {
  if(type == 0)
  {
      build(NODEID_GATEWAY, S_TEMP, C_PRESENTATION, V_LEVEL, 0, 0);
      moSetPayloadType(P_BYTE);
      moSetLength(4);
      sndMsg.payload.data[0] = dht_t/100;
      sndMsg.payload.data[1] = dht_t%100;
      sndMsg.payload.data[2] = dht_h/100;
      sndMsg.payload.data[3] = dht_h%100;    
  }
  else if (type == 1)
  {
      build(NODEID_GATEWAY, S_TEMP, C_PRESENTATION, V_TEMP, 0, 0);
      moSetPayloadType(P_BYTE);
      moSetLength(2);
      sndMsg.payload.data[0] = dht_t/100;
      sndMsg.payload.data[1] = dht_t%100;

  }
  else if (type == 2)
  {
      build(NODEID_GATEWAY, S_HUM, C_PRESENTATION, V_HUM, 0, 0);
      moSetPayloadType(P_BYTE);
      moSetLength(2);
      sndMsg.payload.data[0] = dht_h/100;
      sndMsg.payload.data[1] = dht_h%100; 
  }
  bMsgReady = 1; 
}
#endif