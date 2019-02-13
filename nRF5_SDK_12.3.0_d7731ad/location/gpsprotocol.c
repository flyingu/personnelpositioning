#include "gpsprotocol.h"

uint8_t proto_data_len=0;
uint8_t proto_data_str[255]={0};
uint32_t SN=0x00000001;
//extern uint8_t gpstime;
//extern uint8_t gpstimecount;
extern uint8_t MyGpsId[8];//={0x83,0x60,0x18,0x10,0x03,0x12,0x00,0x01};

uint8_t Getcheck(uint8_t* re,uint8_t len){
    uint8_t t=re[1];
    for(int i=2;i<len-2;i++){
      t=(t^re[i]);
    }
    return t;
}	

void MakeUpCommand(uint8_t* data,uint8_t len,MY_PROTOCOL_TYPE protocoltype){
	
    proto_data_len=0;
    proto_data_str[proto_data_len++]=0xAF;
    for(int i=0;i<8;i++){
      proto_data_str[i+1]=MyGpsId[i];
			proto_data_len++;
    }
    proto_data_str[proto_data_len++]=protocoltype;
    proto_data_str[proto_data_len++]=len;
    for(int i=0;i<len;i++){
      proto_data_str[11+i]=data[i];
			proto_data_len++;
    }
    myint32 ii;
    ii.i32=SN;
    if(SN>=0xffffffff){
            SN=0x00000001;
	}
    for(uint8_t i=0;i<4;i++){
      proto_data_str[proto_data_len++]=ii.arr[3-i];
    }		
    proto_data_str[proto_data_len++]=Getcheck(proto_data_str,len+17);
    proto_data_str[proto_data_len++]=0x3C;
    SN++;		
}
uint8_t ParseData(uint8_t* da2,uint8_t* did,uint16_t len){
	if(da2[0]!=0xAF){
		return 0;
	}       
	uint8_t rl=0;
	for(int i=1;i<9;i++){
		if(da2[i]!=did[i-1]){
			return 0;
		}
	}
	for(int i=9;i<len;i++){
            if(da2[i]==0x3c){
                uint8_t ch=Getcheck(da2,i+1);
                if(ch==da2[i-1]){
                        rl=i+1;
                        break;
                }
            }
	}
	if(rl>0){
          uint8_t type=da2[9];
          //uint8_t datalen=da2[10];
          //uint8_t timedata=da2[11];
         // if(type==PROTOCOL_TYPE_GPS_RESPONSE){
              return type;
        //  } else if              
          //return type;
	}
	return 0;
}

