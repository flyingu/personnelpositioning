#include "sx1278.h" 
#include "nrf_drv_gpiote.h"
#include "main.h"
#include "myflash.h"
//#define PIN_OUT LED_1

#define SPI_PSELMISO0  30    // SPI MISO signal.
#define SPI_PSELSS0    28    // SPI CSN signal.
#define SPI_PSELMOSI0  29    // SPI MOSI signal.
#define SPI_PSELSCK0   00    // SPI SCK signal.
extern locationcard_evt_type_t locationcard_event;
extern flash_store_data_t flash_store_data;
//#define DIO0           6
#define FREQ_STEP             61.03515625
unsigned char   Frequency[3]={0x75,0xe0,0x00};//470
//unsigned char   Frequency[3]={0x6c,0xC0,0x00};//435
//unsigned char   Frequency[3]={0x6c,0x80,0x00};//434
unsigned char   SpreadingFactor=12;           //7-12
unsigned char   CodingRate=2;                 //1-4
unsigned char   Bw_Frequency=7;               //6-9
unsigned char   powerValue=7;
unsigned char   Sx1278_snr=0;
unsigned char   Sx1278_LatestRSSI=0;
unsigned char   Sx1278_CurrentRSSI=0;

unsigned char   power_data[8]={0X81,0X83,0X85,0X87,0X89,0x8b,0x8d,0x8f};
unsigned char   RF_EX0_STATUS;
unsigned char   CRC_Value;
unsigned char   SX1278_RLEN;
uint8_t send_flag;

unsigned char recv[255];
uint8_t recv_length;

void loraTest(void){
  mylorainit(); 
  unsigned char teststr[4]={"1234"};
  SxSend(teststr,0,4);
  for (;;)
  {
      if(recv_length>0){
        recv_length=0;		
      }
  }
}

void SX1278LoRaSetRFFrequency(uint32_t fre)
{
   fre= ( uint32_t )( ( double )fre / ( double )FREQ_STEP );
    Frequency[0] = ( uint8_t )( ( fre >> 16 ) & 0xFF );
    Frequency[1] = ( uint8_t )( ( fre >> 8 ) & 0xFF );
    Frequency[2] = ( uint8_t )( fre & 0xFF );
    SX1276WriteBuffer( REG_LR_FRFMSB, Frequency[0]);
    SX1276WriteBuffer( REG_LR_FRFMID, Frequency[1]);
    SX1276WriteBuffer( REG_LR_FRFLSB, Frequency[2]);
}
void SxSend(uint8_t* str,uint16_t start,uint16_t length)
{
  send_flag=1;
  nrf_gpio_pin_clear(LORA_CTRL);//config switch tx mode
  if(length<=PACK_LEN)
  {
    Sx1276SendPacket(str,0,length);
     while(send_flag>0)//wait send over
    {
    }
  }else
  {
    uint8_t ptr=start;
    uint8_t sendlen=PACK_LEN;
    uint8_t end_p=start+length;
    while(ptr<end_p)
    {
      if((end_p-ptr)<=PACK_LEN)
      {
        sendlen=end_p-ptr;
      }
      Sx1276SendPacket(str,ptr,sendlen);
      while(send_flag>0)//wait send over
      {
      }
      ptr+=sendlen;
      Delay1s(200);
      Delay1s(200);
    }
  }
  nrf_gpio_pin_set(LORA_CTRL);//config switch rx mode
 
}

////Sx1278中断函数入口
//void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){  
//	  SX1278_Interrupt();
//}
/**@snippet [SPI Initialization] */
static void spi_init(void){
	
          nrf_gpio_cfg_output(SPI_PSELSS0);
	  nrf_gpio_cfg_output(SPI_PSELSCK0);
	  nrf_gpio_cfg_output(SPI_PSELMOSI0);
	  nrf_gpio_cfg_input(SPI_PSELMISO0, NRF_GPIO_PIN_PULLUP);	
	
	  nrf_gpio_pin_set(SPI_PSELSS0);
	  nrf_gpio_pin_clear(SPI_PSELSCK0);
	  nrf_gpio_pin_set(SPI_PSELMOSI0);
	 
}
/**@snippet [SPI Initialization] */
static void gpio_init(void){
 //   ret_code_t err_code;

//    err_code = nrf_drv_gpiote_init();
//    APP_ERROR_CHECK(err_code);
    nrf_gpio_cfg_output(18);
    nrf_gpio_pin_clear(18);//config switch tx mode
    nrf_gpio_cfg_output(LORA_RESET);
    nrf_gpio_cfg_output(LORA_CTRL);
    nrf_gpio_pin_set(LORA_RESET);
    nrf_gpio_pin_clear(LORA_CTRL);//config switch tx mode
    //DIO0初始化
//    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true); //GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
//    in_config.pull = NRF_GPIO_PIN_PULLUP;
//
//    err_code = nrf_drv_gpiote_in_init(DIO0, &in_config, in_pin_handler);
//    APP_ERROR_CHECK(err_code);
//
//    nrf_drv_gpiote_in_event_enable(DIO0, true);
}
//LORA复位
void SX1276Reset(void)
{
   nrf_gpio_pin_clear(LORA_RESET);	
   Delay1s(200);
   nrf_gpio_pin_set(LORA_RESET);
   Delay1s(200);
}	
void Delay1s(unsigned int ii)
{
   unsigned char j;
   while(ii--){
     for(j=0;j<100;j++);
   }
}
void RF_SPI_MasterIO(unsigned char out)
{
   unsigned char i;
   for (i=0;i<8;i++){   
     if (out & 0x80)
       nrf_gpio_pin_set(SPI_PSELMOSI0);
     else nrf_gpio_pin_clear(SPI_PSELMOSI0);
		 
     nrf_gpio_pin_set(SPI_PSELSCK0);
     out = (out << 1);
      nrf_gpio_pin_clear(SPI_PSELSCK0);
   }
}


unsigned char RF_SPI_READ_BYTE(void)
{	 
   unsigned char j;
   unsigned char i;
   j=0;
   for (i = 0; i < 8; i++){	 
     nrf_gpio_pin_set(SPI_PSELSCK0); 
     j = (j << 1);
     if( nrf_gpio_pin_read(SPI_PSELMISO0) )
       j = j | 0x01;

      nrf_gpio_pin_clear(SPI_PSELSCK0);
   }
  
   return j;
}

//读寄存器
unsigned char SX1276ReadBuffer(unsigned char addr)
{
  unsigned char Value;
  
  nrf_gpio_pin_clear(SPI_PSELSS0); //NSS = 0;
  RF_SPI_MasterIO( addr & 0x7f  );
  Value = RF_SPI_READ_BYTE();
  nrf_gpio_pin_set(SPI_PSELSS0);//NSS = 1;

  return Value; 
}
//写寄存器
void SX1276WriteBuffer( unsigned char addr, unsigned char buffer)
{ 
   nrf_gpio_pin_clear(SPI_PSELSS0); //NSS = 0;
   RF_SPI_MasterIO( addr | 0x80 );
   RF_SPI_MasterIO( buffer);
    nrf_gpio_pin_set(SPI_PSELSS0); //NSS = 1;
}
//中心频率设置
void SX1276LoRaSetRFFrequency(void)
{
   SX1276WriteBuffer( REG_LR_FRFMSB, Frequency[0]);
   SX1276WriteBuffer( REG_LR_FRFMID, Frequency[1]);
   SX1276WriteBuffer( REG_LR_FRFLSB, Frequency[2]);
}
//设置发射中心频率
void SX1276LoRaSetSendingFrequency(uint8_t *sending_fre)
{
   SX1276WriteBuffer( REG_LR_FRFMSB, sending_fre[0]);
   SX1276WriteBuffer( REG_LR_FRFMID, sending_fre[1]);
   SX1276WriteBuffer( REG_LR_FRFLSB, sending_fre[2]);
}
//发射功率设置
void SX1276LoRaSetRFPower(unsigned char power )
{
   SX1276WriteBuffer( REG_LR_PADAC, 0x87);
   SX1276WriteBuffer( REG_LR_PACONFIG,  power_data[power] );
}
//扩频因子设置
void SX1276LoRaSetSpreadingFactor(unsigned char factor )
{
   unsigned char RECVER_DAT;
   SX1276LoRaSetNbTrigPeaks( 3 );
   RECVER_DAT=SX1276ReadBuffer( REG_LR_MODEMCONFIG2);	  
   RECVER_DAT = ( RECVER_DAT & RFLR_MODEMCONFIG2_SF_MASK ) | ( factor << 4 );
   SX1276WriteBuffer( REG_LR_MODEMCONFIG2, RECVER_DAT );	 
}
//编码率设置
void SX1276LoRaSetErrorCoding(unsigned char value )
{	
   unsigned char RECVER_DAT;
   RECVER_DAT=SX1276ReadBuffer( REG_LR_MODEMCONFIG1);
   RECVER_DAT = ( RECVER_DAT & RFLR_MODEMCONFIG1_CODINGRATE_MASK ) | ( value << 1 );
   SX1276WriteBuffer( REG_LR_MODEMCONFIG1, RECVER_DAT);
   // LoRaSettings.ErrorCoding = value;
}
//数据CRC校验设置
void SX1276LoRaSetPacketCrcOn(BOOL enable )
{	
   unsigned char RECVER_DAT;
   RECVER_DAT= SX1276ReadBuffer( REG_LR_MODEMCONFIG2);
   RECVER_DAT = ( RECVER_DAT & RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK ) | ( enable << 2 );
   SX1276WriteBuffer( REG_LR_MODEMCONFIG2, RECVER_DAT );
}
//LORA模式设置
void SX1276LoRaSetOpMode( RFMode_SET opMode )
{
   unsigned char opModePrev;
   opModePrev=SX1276ReadBuffer(REG_LR_OPMODE);
   opModePrev &=0xf8;
   opModePrev |= (unsigned char)opMode;
   SX1276WriteBuffer( REG_LR_OPMODE, opModePrev);		
}
//FSK or LORA模式设置
void SX1276LoRaFsk( Debugging_fsk_ook opMode )
{
   unsigned char opModePrev;
   opModePrev=SX1276ReadBuffer(REG_LR_OPMODE);
   opModePrev &=0x7F;
   opModePrev |= (unsigned char)opMode;
   SX1276WriteBuffer( REG_LR_OPMODE, opModePrev);		
}
//带宽设置
void SX1276LoRaSetSignalBandwidth(unsigned char bw )
{
   unsigned char RECVER_DAT;
   RECVER_DAT=SX1276ReadBuffer( REG_LR_MODEMCONFIG1);
   RECVER_DAT = ( RECVER_DAT & RFLR_MODEMCONFIG1_BW_MASK ) | ( bw << 4 );
   SX1276WriteBuffer( REG_LR_MODEMCONFIG1, RECVER_DAT );
   // LoRaSettings.SignalBw = bw;
}
//报头模式设置
void SX1276LoRaSetImplicitHeaderOn(BOOL enable )
{
   unsigned char RECVER_DAT;
   RECVER_DAT=SX1276ReadBuffer( REG_LR_MODEMCONFIG1 );
   RECVER_DAT = ( RECVER_DAT & RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK ) | ( enable );
   SX1276WriteBuffer( REG_LR_MODEMCONFIG1, RECVER_DAT );
}
//负载长度设置
void SX1276LoRaSetPayloadLength(unsigned char value )
{
   SX1276WriteBuffer( REG_LR_PAYLOADLENGTH, value );
} 
void SX1276LoRaSetSymbTimeout(unsigned int value )
{
   unsigned char RECVER_DAT[2];
   RECVER_DAT[0]=SX1276ReadBuffer( REG_LR_MODEMCONFIG2 );
   RECVER_DAT[1]=SX1276ReadBuffer( REG_LR_SYMBTIMEOUTLSB );
   RECVER_DAT[0] = ( RECVER_DAT[0] & RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK ) | ( ( value >> 8 ) & ~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK );
   RECVER_DAT[1] = value & 0xFF;
   SX1276WriteBuffer( REG_LR_MODEMCONFIG2, RECVER_DAT[0]);
   SX1276WriteBuffer( REG_LR_SYMBTIMEOUTLSB, RECVER_DAT[1]);
}

void SX1276LoRaSetPreamLength(unsigned int value )
{
   unsigned char a[2];
   a[0]=(value&0xff00)>>8;
   a[1]=value&0xff;
   SX1276WriteBuffer( REG_LR_PREAMBLEMSB, a[0] );
   SX1276WriteBuffer( REG_LR_PREAMBLELSB, a[1] );
}
//低数据优化
void SX1276LoRaSetMobileNode(BOOL enable )
{	 
   unsigned char RECVER_DAT;
   RECVER_DAT=SX1276ReadBuffer( REG_LR_MODEMCONFIG3 );
   RECVER_DAT = ( RECVER_DAT & RFLR_MODEMCONFIG3_MOBILE_NODE_MASK ) | ( enable << 3 );
   SX1276WriteBuffer( REG_LR_MODEMCONFIG3, RECVER_DAT );
}
void Sx1276SendPacket(unsigned char *RF_TRAN_P,unsigned char START,unsigned char LEN)
{
   unsigned char ASM_i;
   //lpTypefunc.paSwitchCmdfunc(txOpen);
   SX1276LoRaSetOpMode( Stdby_mode );
   SX1276WriteBuffer( REG_LR_HOPPERIOD, 0 );	//不做频率跳变
   SX1276WriteBuffer(REG_LR_IRQFLAGSMASK,IRQN_TXD_Value);	//打开发送中断
   SX1276WriteBuffer( REG_LR_PAYLOADLENGTH, LEN);	 //最大数据包
   SX1276WriteBuffer( REG_LR_FIFOTXBASEADDR, 0);
   SX1276WriteBuffer( REG_LR_FIFOADDRPTR, 0 );
   nrf_gpio_pin_clear(SPI_PSELSS0); //NSS = 0;
   RF_SPI_MasterIO( 0x80 );//lpTypefunc.lpByteWritefunc( 0x80 );
   RF_TRAN_P+=START;
   for( ASM_i = 0x00; ASM_i < LEN; ASM_i++ ){
     RF_SPI_MasterIO( *RF_TRAN_P );//lpTypefunc.lpByteWritefunc( *RF_TRAN_P );
     RF_TRAN_P++;
   }
   nrf_gpio_pin_set(SPI_PSELSS0); //NSS = 1;
   SX1276WriteBuffer(REG_LR_DIOMAPPING1,0x40);
   SX1276WriteBuffer(REG_LR_DIOMAPPING2,0x00);
   SX1276LoRaSetOpMode( Transmitter_mode );
}


void Sx1276Receive (void)
{
   SX1276LoRaSetOpMode(Stdby_mode );
   SX1276WriteBuffer(REG_LR_IRQFLAGSMASK,IRQN_RXD_Value);  //打开发送中断
   SX1276WriteBuffer(REG_LR_HOPPERIOD,	PACKET_MIAX_Value );
   SX1276WriteBuffer( REG_LR_DIOMAPPING1, 0X00 );
   SX1276WriteBuffer( REG_LR_DIOMAPPING2, 0X00 );	
   SX1276LoRaSetOpMode( Receiver_mode );
   //lpTypefunc.paSwitchCmdfunc(rxOpen);
}
void Sx1276Sleep(void)
{
   SX1276LoRaSetOpMode( Stdby_mode );
   SX1276WriteBuffer(REG_LR_IRQFLAGSMASK,  IRQN_SEELP_Value);  //打开发送中断
   SX1276WriteBuffer( REG_LR_DIOMAPPING1, 0X00 );
   SX1276WriteBuffer( REG_LR_DIOMAPPING2, 0X00 );	
   SX1276LoRaSetOpMode( Sleep_mode );
}
void Sx1276CadReceive (void)
{
   SX1276LoRaSetOpMode( Stdby_mode );
   SX1276WriteBuffer(REG_LR_IRQFLAGSMASK,  IRQN_CAD_Value);	//打开发送中断
   SX1276WriteBuffer( REG_LR_DIOMAPPING1, 0X80 );
   SX1276WriteBuffer( REG_LR_DIOMAPPING2, 0X00 );	
   SX1276LoRaSetOpMode( CAD_mode );
   //lpTypefunc.paSwitchCmdfunc(rxOpen);
}

//LORA初始化
void SX1276LORA_INT(void)
{
   SX1276LoRaSetOpMode(Sleep_mode);  //设置睡眠模式
   SX1276LoRaFsk(LORA_mode);	     // 设置扩频模式
   SX1276LoRaSetOpMode(Stdby_mode);  // 设置为普通模式
   SX1276WriteBuffer( REG_LR_DIOMAPPING1,GPIO_VARE_1);
   SX1276WriteBuffer( REG_LR_DIOMAPPING2,GPIO_VARE_2);
   SX1276LoRaSetRFFrequency();
   SX1276LoRaSetRFPower(powerValue);
   SX1276LoRaSetSpreadingFactor(SpreadingFactor);	 // 扩频因子设置
   SX1276LoRaSetErrorCoding(CodingRate);		 //有效数据比
   SX1276LoRaSetPacketCrcOn(true);			 //CRC 校验打开
   SX1276LoRaSetSignalBandwidth( Bw_Frequency );	 //设置扩频带宽
   SX1276LoRaSetImplicitHeaderOn(false);		 //同步头是显性模式
   SX1276LoRaSetPayloadLength( 0xff);
   SX1276LoRaSetSymbTimeout( 0x3FF );
  // SX1276LoRaSetPreamLength(6);
   SX1276LoRaSetMobileNode(true); 			 // 低数据的优化
   
   //SX1276LoRaSetPreamLength(0x06);
  Sx1276Receive();
   //Sx1276Sleep();
}
void mylorainit(void){
	
	spi_init();
	gpio_init();
	SX1276Reset();
        Delay1s(200);
        SX1276LORA_INT();
        Delay1s(200);
        Delay1s(200);		
}
void mylora_sending_init(void){
	
        spi_init();
        gpio_init();
        SX1276Reset();
        Delay1s(200);
        
      //  SX1276LORA_INT();
       SX1276LoRaSetOpMode(Sleep_mode);  //设置睡眠模式
       SX1276LoRaFsk(LORA_mode);	 //设置扩频模式
       SX1276LoRaSetOpMode(Stdby_mode);  //设置为普通模式
       SX1276WriteBuffer( REG_LR_DIOMAPPING1,GPIO_VARE_1);
       SX1276WriteBuffer( REG_LR_DIOMAPPING2,GPIO_VARE_2);
       SX1276LoRaSetSendingFrequency(flash_store_data.flash_store_data_s.fre);
       SX1276LoRaSetRFPower(flash_store_data.flash_store_data_s.power);
       SX1276LoRaSetSpreadingFactor(flash_store_data.flash_store_data_s.sf);	 // 扩频因子设置
       SX1276LoRaSetErrorCoding(flash_store_data.flash_store_data_s.cr);		 //有效数据比
       SX1276LoRaSetPacketCrcOn(true);			 //CRC 校验打开
       SX1276LoRaSetSignalBandwidth( flash_store_data.flash_store_data_s.bw );	 //设置扩频带宽
       SX1276LoRaSetImplicitHeaderOn(false);		 //同步头是显性模式
       SX1276LoRaSetPayloadLength( 0xff);
       SX1276LoRaSetSymbTimeout( 0x3FF );
      // SX1276LoRaSetPreamLength(6);
       SX1276LoRaSetMobileNode(true); 			 // 低数据的优化   
       //SX1276LoRaSetPreamLength(0x06);
      //Sx1276Receive();
      // Sx1276Sleep();   
        Delay1s(200);
        Delay1s(200);		
}
void mylora_receive_init(void){
	
        spi_init();
        gpio_init();
        SX1276Reset();
        Delay1s(200);
        
      //  SX1276LORA_INT();
       SX1276LoRaSetOpMode(Sleep_mode);  //设置睡眠模式
       SX1276LoRaFsk(LORA_mode);	 //设置扩频模式
       SX1276LoRaSetOpMode(Stdby_mode);  //设置为普通模式
       SX1276WriteBuffer( REG_LR_DIOMAPPING1,GPIO_VARE_1);
       SX1276WriteBuffer( REG_LR_DIOMAPPING2,GPIO_VARE_2);
       SX1276LoRaSetSendingFrequency(flash_store_data.flash_store_data_s.fre);
       SX1276LoRaSetRFPower(flash_store_data.flash_store_data_s.power);
       SX1276LoRaSetSpreadingFactor(flash_store_data.flash_store_data_s.sf);	 // 扩频因子设置
       SX1276LoRaSetErrorCoding(flash_store_data.flash_store_data_s.cr);		 //有效数据比
       SX1276LoRaSetPacketCrcOn(true);			 //CRC 校验打开
       SX1276LoRaSetSignalBandwidth( flash_store_data.flash_store_data_s.bw );	 //设置扩频带宽
       SX1276LoRaSetImplicitHeaderOn(false);		 //同步头是显性模式
       SX1276LoRaSetPayloadLength(0xff);
       SX1276LoRaSetSymbTimeout(0x3FF);
      // SX1276LoRaSetPreamLength(6);
       SX1276LoRaSetMobileNode(true); 			 // 低数据的优化   
       //SX1276LoRaSetPreamLength(0x06);
      //Sx1276Receive();
      // Sx1276Sleep();   
        Delay1s(200);
        Delay1s(200);		
}
void SX1278_Interrupt(void)
{
  RF_EX0_STATUS=SX1276ReadBuffer( REG_LR_IRQFLAGS ); 
  if((RF_EX0_STATUS&0x40)==0x40){//RX done
    CRC_Value=SX1276ReadBuffer( REG_LR_MODEMCONFIG2 );
    if((CRC_Value&0x04)==0x04){
      SX1276WriteBuffer (REG_LR_FIFOADDRPTR,0x00);
      SX1278_RLEN = SX1276ReadBuffer(REG_LR_NBRXBYTES);
      nrf_gpio_pin_clear(SPI_PSELSS0);// nss=0
      RF_SPI_MasterIO( 0x00 );
      for(unsigned char RF_REC_RLEN_i=0;RF_REC_RLEN_i<SX1278_RLEN;RF_REC_RLEN_i++){
        recv[RF_REC_RLEN_i]=RF_SPI_READ_BYTE();
      }
      recv_length=SX1278_RLEN;
      Sx1278_snr=getSX1276PacketSnr();
      Sx1278_LatestRSSI=getSX1276LatestRSSI();
      Sx1278_CurrentRSSI=getSX1276CurrentRSSI();
      nrf_gpio_pin_set(SPI_PSELSS0);
    }   
    Sx1276Sleep();
  }else if((RF_EX0_STATUS&0x08)==0x08){//TX done
      mylorainit();
     Sx1276Receive();
     //Sx1276Sleep();
      send_flag=0;
     // locationcard_event=LORA_REV_DONE_EVT;
   
  }else if((RF_EX0_STATUS&0x04)==0x04){  
    if((RF_EX0_STATUS&0x01)==0x01){     //表示CAD 检测到扩频信号 模块进入了接收状态来接收数据
     Sx1276Sleep();
    }else{                           // 没检测到
      SX1276LoRaSetOpMode( Stdby_mode );
      SX1276WriteBuffer(REG_LR_IRQFLAGSMASK,  IRQN_SEELP_Value); 
      SX1276WriteBuffer( REG_LR_DIOMAPPING1, 0X00 );
      SX1276WriteBuffer( REG_LR_DIOMAPPING2, 0X00 );	
      SX1276LoRaSetOpMode( Sleep_mode );
      //PA_SEELP_OUT();
    }
  }else{
   Sx1276Sleep();
  }
  SX1276WriteBuffer( REG_LR_IRQFLAGS, 0xff  ); 
}
void SX1276LoRaSetNbTrigPeaks(unsigned char value )
{
   unsigned char RECVER_DAT;
   RECVER_DAT = SX1276ReadBuffer( 0x31);
   RECVER_DAT = ( RECVER_DAT & 0xF8 ) | value;
   SX1276WriteBuffer( 0x31, RECVER_DAT );
}

 unsigned char getSX1276PacketSnr(void)
{
  unsigned char Value;
  
//  HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin,GPIO_PIN_RESET); //NSS = 0;
//  RF_SPI_MasterIO( 0x19 & 0x7f  );
//  Value = RF_SPI_READ_BYTE();
//  HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin,GPIO_PIN_SET);
  Value=SX1276ReadBuffer(0x19);
  return Value; 
}

unsigned char getSX1276LatestRSSI(void)
{
  unsigned char Value;
  
//  HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin,GPIO_PIN_RESET); //NSS = 0;
//  RF_SPI_MasterIO( 0x1a & 0x7f  );
//  Value = RF_SPI_READ_BYTE();
//  HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin,GPIO_PIN_SET); 
  Value=SX1276ReadBuffer(0x1a);

  return Value; 
}
unsigned char getSX1276CurrentRSSI(void)
{
  unsigned char Value;
//  
//  HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin,GPIO_PIN_RESET); //NSS = 0;
//  RF_SPI_MasterIO( 0x1b & 0x7f  );
//  Value = RF_SPI_READ_BYTE();
//  HAL_GPIO_WritePin(NSS_GPIO_Port, NSS_Pin,GPIO_PIN_SET); 
   Value=SX1276ReadBuffer(0x1b);

  return Value; 
}
void Sx1278_Init(uint32_t fre){
  spi_init();
  gpio_init();
  SX1276Reset();
  Delay1s(200);
  SX1276LoRaSetOpMode(Sleep_mode);  //设置睡眠模式
  SX1276LoRaFsk(LORA_mode);	     // 设置扩频模式
  SX1276LoRaSetOpMode(Stdby_mode);  // 设置为普通模式
  SX1276WriteBuffer( REG_LR_DIOMAPPING1,GPIO_VARE_1);
  SX1276WriteBuffer( REG_LR_DIOMAPPING2,GPIO_VARE_2);
  SX1278LoRaSetRFFrequency(fre);
  SX1276LoRaSetRFPower(powerValue);
  SX1276LoRaSetSpreadingFactor(SpreadingFactor);	 // 扩频因子设置
  SX1276LoRaSetErrorCoding(CodingRate);		 //有效数据比
  SX1276LoRaSetPacketCrcOn(true);			 //CRC 校验打开
  SX1276LoRaSetSignalBandwidth( Bw_Frequency );	 //设置扩频带宽
  SX1276LoRaSetImplicitHeaderOn(false);		 //同步头是显性模式
  SX1276LoRaSetPayloadLength( 0xff);
  SX1276LoRaSetSymbTimeout( 0x3FF );
  // SX1276LoRaSetPreamLength(6);
  SX1276LoRaSetMobileNode(true); 			 // 低数据的优化 
  //SX1276LoRaSetPreamLength(0x06);
  Sx1276Receive();
 // Sx1276Sleep();
  Delay1s(200);
  Delay1s(200);	

} 



