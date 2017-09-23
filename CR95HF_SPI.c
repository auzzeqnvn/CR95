/* Includes ------------------------------------------------------------------*/
#include "CR95HF_SPI.h"
#include "stm32f30x_spi.h"
#include "stm32f30x.h"
#include "main.h"
#include "stdio.h"
#include "stm32f30x_iwdg.h"
#include "stm32f30x_gpio.h"
#include <stdlib.h>
#include <cstdlib> 

uint8_t buff_id[3];
uint8_t buff_GPLX[18],buff_ten_lx[45],buff_sdt_lx[20];
uint8_t buff_write_data[100];
extern uint8_t buff_data_tag[100],buff_data_notag[100];//buff gui data len H2, 77 byte

uint8_t flag_on_power=1,flag_wakeup=0,timer_ECH_cr95hf=0,flag_ECH_cr95hf=0,num_wakeup_cr95hf=0;
uint8_t flag_read_rfid=0,num_info=0;
uint8_t num_read_again=0,timer_check_tag=0,flag_write_data=0;
uint8_t flag_reset_cr95hf=0,timer_resset_cr95hf=0,num_notag_cr95hf=0,timer_wakeup_cr5hf=0;
uint8_t num_ech_err=0,num_infotag_err=0,num_notag=0,num=0,flag_write_again=0,flag_read_again=0,timer_wait_poling=0;
uint8_t num_readtag_again=0,num_read_err=0,flag_check_tag=0,num_write_again=0;
extern uint8_t index_GPLX_cr95hf,index_ten_lx_cr95hf,index_sdt_lx_cr95hf,timer_send_RFID,index_data_RF,num_notag_reset;
extern uint8_t timer_on_power_cr95hf,status_old_tag,status_tag,status_cr95hf,led_tag,status_active_cr95hf;
extern uint8_t num_send_status_rf,status_comRFID,num_buzz,flag_debug,flag_start_send_data_firm,init_cr95hf_ok;

extern uint8_t buff_id_cr95hf[5],buff_data_RF[100];

char command_ECH[3]={0x00,0x55,0x00};
char command_IDN[3]={0x00,0x01,0x00};
//ISO15693
char command_156_ISO[5]={0x00,0x02,0x02,0x01,0x0D};
char command_156_INFO[5]={0x00,0x04,0x02,0x02,0x2B};
char command_READ_TAG[6]={0x00,0x04,0x03,0x02,0x20,0x00};
char command_RESET[2]={0x00,0x01};
char command_156_GAIN[17]={0x00,0x07,0x0E,0x03,0xA1,0x00,0xF8,0x01,0x18,0x00,0x20,0x60,0x60,0x64,0x74,0x3F,0x01};
char command_STC[6]={0x00,0x08,0x03,0x62,0x01,0x00};


void sys_CR95HF(void)
{
	if(flag_on_power==1 && timer_on_power_cr95hf>1){
		timer_on_power_cr95hf=0;		
		GPIO_WriteBit(GPIO_RFID_EN,RFID_EN, (BitAction)(1));
		flag_check_tag=1;
		flag_on_power=0;
		flag_wakeup=1;
		num_wakeup_cr95hf=0;
		timer_wakeup_cr5hf=0;
	}
	if(flag_wakeup==1){//wakeup xong moi init-doc-ghi
		Wakeup_CR95HF();
	}
	else{
		if(timer_ECH_cr95hf>=200 && flag_ECH_cr95hf==1){
			ECH_CR95HF();
		}     
		if(flag_read_rfid==1&& init_cr95hf_ok==1){//
			flag_read_rfid=0;
			if(status_tag==1){//&& status_write_data==0
				Read_Data_Tag();
			}
		}
		if(flag_check_tag==1 && timer_check_tag>=1 && init_cr95hf_ok==1 && flag_start_send_data_firm==0){// && flag_check_tag==1&& status_write_data==0
			timer_check_tag=0;
			flag_check_tag=0;
			TAG_INFO();
			timer_check_tag=0;
			flag_check_tag=1;
		}
		if(flag_write_data==1 && status_tag==1){
			flag_write_data=0;
			write_secter(buff_write_data);
		}
	}
	RESET_CR95HF();
	if(num_notag_reset>100){
    num_notag_reset=0;
	  flag_reset_cr95hf=1;
  }
}


void CR95HF_Write(uint8_t *buff, uint8_t length)
{
	uint8_t i=0,dau_phay=0,checksum=0,numbyte=0,index_lx=0;
	for(i=0;i<77;i++)	buff_write_data[i]=0;
	for(i=0;i<length-1;i++)
	{//data    (4)   
		if(buff[i]==0x2C)
		{
			dau_phay++;
			if(dau_phay==1)
			{//lay so byte ten lx, ghi sau gplx de neu index_gplx >16 thi ko bi mat ten lx
				buff_write_data[59]=checksum;
				checksum=0;
				numbyte=0;
				index_lx=i;
			}
			if(dau_phay==2){//them gplx lai xe vao byte 0 -> byte (j-3)
				checksum=checksum-buff_write_data[i-index_lx-1];
				buff_write_data[i-index_lx-1]=0;
				buff_write_data[15]=checksum;
				checksum=0;
				numbyte=0;
				index_lx=i;
			}
		}
		if(dau_phay<1){// add ten lx, 43 byte, byte 44 la crc
			buff_write_data[i+16]=buff[i];//+16-3=13
			numbyte++;
			if(numbyte<=43){
				checksum=checksum+buff_write_data[i+16];
			}  
		}
		if(dau_phay>=1 && dau_phay<2){// add gplx 15 byte, byte 16 la crc
			buff_write_data[i-index_lx]=buff[i+1];
			numbyte++;
			if(numbyte<=15){
				checksum=checksum+buff_write_data[i-index_lx];
			}         
		}
		if(dau_phay>=2 && i<(length-2)){// add 12 byte sdt, tinh crc sdt la byte 13
			buff_write_data[i-index_lx+60]=buff[i+1];         
			numbyte++;
		 if(numbyte<=12){
				checksum=checksum+buff_write_data[i-index_lx+60];
			} 
		}
	}
	buff_write_data[72]=checksum;// check sum sdt  // byte 73-> 75 ko co data, them 76 byte vao de ghi cho du 1 secter
	flag_write_data=1;
	num_write_again=0;
}
void write_secter(uint8_t *buff)
{
	uint8_t i=0,index=0,num_write=0,j=0,num_write_err=0;
	uint16_t num_wait=0;

	uint8_t data[5];
	char temp_buff[10];

  CR95HF_deselect();
  temp_buff[0]=0x00;
  temp_buff[1]=0x04;
  temp_buff[2]=0x07;
  temp_buff[3]=0x02;
  temp_buff[4]=0x21;
  for(i=0;i<19;i++)
	{
    temp_buff[5]=i;
    temp_buff[6]=buff[index];
    temp_buff[7]=buff[index+1];
    temp_buff[8]=buff[index+2];
    temp_buff[9]=buff[index+3];   
    data[0]=0;
    SPI_SendCommand_CR95HF(temp_buff,10); 
		if(wait_polling(2)==1)
		{
			timer_check_tag=0;
		}
    data[0]=0;
    CR95HF_select();
      CR95HF_transfer(0x02);  
      data[0]=CR95HF_transfer(0);//response
      data[1]=CR95HF_transfer(0);//length data
      for(j=0;j<data[1];j++){
        CR95HF_transfer(0);
      }
      if(data[0]!=0x80){//ghi err
        i--;
        num_write_err++;
        if(num_write_err>50){
          num_write_err=0;
          num_write_again++;
          flag_write_again=1;
        //  status_write_data=0;
          i=20;         
          timer_check_tag=5;
          if(num_write_again>2){
            num_write_again=0;
            flag_write_data=0;
          }
        }
      }
      else{
        num_write_err=0;
        num_write_again=0;
        num_write++;
        index=index+4;
      }
    CR95HF_deselect();
  }
  if(num_write==19){
    num_write=0;
    flag_write_data=0;
    num_buzz=2;
  }
}
void RESET_CR95HF(void){
	if(flag_reset_cr95hf==1){
	//	CR95HF_select();
	//	CR95HF_transfer(0x01);
	 // printf(" -- reset cr95hf2");  
	//	CR95HF_deselect();
		flag_reset_cr95hf=2;
		flag_ECH_cr95hf=0;
		init_cr95hf_ok=0;
		status_cr95hf=ERROR;
		timer_resset_cr95hf=0;
		num_notag_cr95hf=0;
		status_tag=ERROR;
		init_cr95hf_ok=0;
		status_old_tag=0;
		num_notag_reset=0;
		GPIO_WriteBit(GPIO_RFID_EN,RFID_EN, (BitAction)(0));
		CR95HF_deselect();
	}
	if(timer_resset_cr95hf>=2 && flag_reset_cr95hf==2){
		flag_reset_cr95hf=0;
		GPIO_WriteBit(GPIO_RFID_EN,RFID_EN, (BitAction)(1));
		flag_on_power=1;
		timer_on_power_cr95hf=0;
		init_cr95hf_ok=0;
		timer_wakeup_cr5hf=0;
	}
}
void ECH_CR95HF(void){
  uint8_t data=0,flag_ech_err=0,i=0;
 // u16 num_wait=0;
  SPI_SendCommand_CR95HF(command_ECH,3);
  if(wait_polling(1)==1){
		 timer_ECH_cr95hf=0;
		 flag_ECH_cr95hf=1;
		 flag_ech_err=1;    
  }
  if(flag_ech_err==0){ 
    CR95HF_select();
    CR95HF_transfer(0x02);    
    data=CR95HF_transfer(0);  
    CR95HF_deselect();
    if(data==0x55){
      if(IDN_CR95HF()==1){
        ISO156_CR95HF();
      }  
    }
    else{  
      timer_ECH_cr95hf=0;
      flag_ECH_cr95hf=1;
   //   for(char i=0;i<100;i++){
   //     SPI_SendCommand_CR95HF(command_ECH,3);
   //   }
      num_ech_err++;
      if(num_ech_err>5){
        num_ech_err=0;
        flag_reset_cr95hf=1;
      }  
    }
  }
  else{//ech lai
    flag_ech_err=0;
    for(i=0;i<200;i++){
      SPI_SendCommand_CR95HF(command_ECH,3);
    }
    num_ech_err++;
    if(num_ech_err>5){
      num_ech_err=0;
      flag_reset_cr95hf=1;
    }
  }
}
char wait_polling(char timer_poling){
  uint8_t poll=0,data=0;
  uint16_t num_wait=0;
  CR95HF_select();
  timer_wait_poling=0;
	timer_poling=timer_poling*3;
	while(data!=0x08){
		if(timer_wait_poling>=timer_poling){
			timer_wait_poling=0;
			data= CR95HF_transfer(0x03);
			data=data&0x08;
			num_wait++;
			IWDG_ReloadCounter();
			if(num_wait>=20){
				data=0x08;
				poll=1;   
				CR95HF_deselect();
				num_wait=0;
			}     
		}
	}
  CR95HF_deselect();
  return poll;
}
char IDN_CR95HF(void){
  char data=0,i;
  SPI_SendCommand_CR95HF(command_IDN,3);
  if(wait_polling(1)==1){
       timer_ECH_cr95hf=0;
       flag_ECH_cr95hf=1;
       return 0;   
  } 
  CR95HF_select();
  CR95HF_transfer(0x02);  
  CR95HF_transfer(0);//response
  data=CR95HF_transfer(0);//len data
  for(i=0;i<data;i++){
    CR95HF_transfer(0);
  }  
  CR95HF_deselect();
  if(data==0x0F){ 
    return 1;    
  }
  else{ 
    timer_ECH_cr95hf=0;
    flag_ECH_cr95hf=1;
    return 0;   
  }
}
char ISO156_CR95HF(void){
  char data=0;
//  u16 num_wait=0;
  SPI_SendCommand_CR95HF(command_156_ISO,5);
  if(wait_polling(1)==1){
      timer_ECH_cr95hf=0;
      flag_ECH_cr95hf=1;
      return 0;    
  }  
  CR95HF_select();
    CR95HF_transfer(0x02);  
    CR95HF_transfer(0);//response
    data=CR95HF_transfer(0);//len data  
  CR95HF_deselect();
  if(data==0x00){
    init_cr95hf_ok=1;
    flag_ECH_cr95hf=0;
		status_cr95hf=OK;
    return 1;    
  }
  else{
    timer_ECH_cr95hf=0;
    flag_ECH_cr95hf=1;
    return 0;   
  }
}
void TAG_INFO(void){
  uint8_t i=0;
  uint8_t data[20];
  data[0]=0;
  SPI_SendCommand_CR95HF(command_156_INFO,5); 
  if(wait_polling(1)==1){
    timer_check_tag=0;
  }
  CR95HF_select();
	CR95HF_transfer(0x02);  
	data[0]=CR95HF_transfer(0);//response	
	data[1]=CR95HF_transfer(0);//len data
	num_info++;
	if(data[1]<20){//kt tran
		for(i=0;i<data[1];i++){//
			data[i+2]=CR95HF_transfer(0);
		}
	}
  CR95HF_deselect(); 
	
  if(data[0]==0x80){  
    timer_check_tag=0;
    status_tag=OK;
    num_infotag_err=0;
    num_notag_reset=0;
    if(flag_read_again==1){
      flag_read_again=0;
      flag_read_rfid=1;
    }
    if(flag_write_again==1){
      flag_write_again=0;
      flag_write_data=1;
    }
    timer_check_tag=0;   
  }
  else{
		num_notag_reset++;
    if(data[0]==0x87 || data[0]==0x88){ 
      num_infotag_err=0;
    }
    else{
			if(data[0]==0xFF){
				num_infotag_err=0;
				flag_reset_cr95hf=1;
			}
			else{
				num_infotag_err++;
	//      timer_check_tag=0;
				if(num_infotag_err>5){//20
					num_infotag_err=0;
					flag_reset_cr95hf=1;
				}
			}
    }
		status_tag=ERROR;
		status_old_tag=0;
    timer_check_tag=0;
  }
	if((status_old_tag==0 && status_tag==OK)){
		if(status_old_tag==0){
			status_old_tag=1;
		}
		flag_read_rfid=1;
		timer_send_RFID=0;
	}
}
void Read_Data_Tag(void)
{ 
	uint8_t addres_cr95hf=0,i=0,k=0;
	uint16_t num_wait=0;
	uint8_t data[20];
	
	index_GPLX_cr95hf=0;
	index_ten_lx_cr95hf=0;
	index_sdt_lx_cr95hf=0;
	
	// Clear buff_data_tag
	for(i=0;i<80;i++)	buff_data_tag[i]=0;
	// Read sector 0->19
	for(addres_cr95hf=0;addres_cr95hf<19;addres_cr95hf++)
	{
		command_READ_TAG[5]=addres_cr95hf;
		SPI_SendCommand_CR95HF(command_READ_TAG,6);
		data[0]=0;
		CR95HF_select();
		timer_wait_poling=0;
      while(data[0]!=0x08)
		{//wait poling
			if(timer_wait_poling>=2)
			{
				timer_wait_poling=0;
				data[0]= CR95HF_transfer(0x03);
				data[0]=data[0]&0x08;
				num_wait++;
				if(num_wait>=400)
				{
					num_wait=0;  
					data[0]=0x08;//thoat while
					addres_cr95hf=30;//thoat khoi for
					flag_check_tag=1;//vao kiem tra the      
				}
			}
      }
      CR95HF_deselect();
      data[0]=0;
      CR95HF_select();
      CR95HF_transfer(0x02);  
      data[0]=CR95HF_transfer(0);//response
      if(data[0]==0x80 )
		{//data the tra ve || data[0]==0x8e
        data[1]=CR95HF_transfer(0);//len data
        if(data[1]<20)
		  {
          for(i=0;i<data[1];i++)	data[i+2]=CR95HF_transfer(0);
          CR95HF_transfer(0);
          CR95HF_transfer(0);
          CR95HF_transfer(0);
        }
        if((addres_cr95hf>=add_gplx_start) && (addres_cr95hf<=add_gplx_end))
		  {
				for(k=0;k<4;k++)
				{
					buff_data_tag[index_GPLX_cr95hf]=data[k+3];         
					index_GPLX_cr95hf++;
				}
			}
			if((addres_cr95hf>=add_tenlx_start) && (addres_cr95hf<=add_tenlx_end))
			{//data ten lx va sdt
				for(k=0;k<4;k++)
				{
					buff_data_tag[index_ten_lx_cr95hf+16]=data[k+3];
					index_ten_lx_cr95hf++;
					
				}
			}
			if((addres_cr95hf>=add_sdtlx_start) && (addres_cr95hf<=add_sdtlx_end))
			{//data ten lx va sdt
				for(k=0;k<4;k++)
				{
					buff_data_tag[index_sdt_lx_cr95hf+64]=data[k+3];
					index_sdt_lx_cr95hf++;
				}
			}
        num_read_err=0;
      }
      else
		{// 
			addres_cr95hf=30;      
			num_readtag_again++;
			if(num_readtag_again>2)	num_readtag_again=0;
			else	flag_read_again=1;
      }
		CR95HF_deselect(); 
	}
	if(addres_cr95hf==19)
	{
		buff_data_tag[60]=buff_id_cr95hf[0];
		buff_data_tag[61]=buff_id_cr95hf[1];
		buff_data_tag[62]=buff_id_cr95hf[2];
		buff_data_tag[63]=status_active_cr95hf;

		flag_read_rfid=0;
		addres_cr95hf=0;
		index_data_RF=77;
		for(i=0;i<77;i++)	buff_data_RF[i]=buff_data_tag[i];
		num_send_status_rf=0;
		status_comRFID=OK;
		xuly_data_rfid(buff_data_RF,index_data_RF);
		index_data_RF=0;
	}
}

void Wakeup_CR95HF(void)
{
	if(num_wakeup_cr95hf==0 && timer_wakeup_cr5hf>5){
		timer_wakeup_cr5hf=0;
		num_wakeup_cr95hf=1;
		GPIO_WriteBit(GPIO_RFID_IRQ,RFID_IRQ, (BitAction)(0));
	}
  if(timer_wakeup_cr5hf>10 && num_wakeup_cr95hf==1){
    timer_wakeup_cr5hf=0;
		num_wakeup_cr95hf=0;
    GPIO_WriteBit(GPIO_RFID_IRQ,RFID_IRQ, (BitAction)(1));
    flag_wakeup=0;
    timer_ECH_cr95hf=0;
    flag_ECH_cr95hf=1;
  }
}
/*
*	Init SPI interface for CR95
*/
void INIT_SPI2(void)
{
	SPI_InitTypeDef   SPI_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	char dummyread;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SPI_CR95HF_SCK | GPIO_Pin_SPI_CR95HF_MOSI | GPIO_Pin_SPI_CR95HF_MISO; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_Init (GPIO_SPI_CR95HF, & GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SPI_CR95HF_CS ; 
	GPIO_Init (GPIO_SPI_CR95HF, & GPIO_InitStructure); 
	
	RCC_APB1PeriphClockCmd (RCC_SPI_CR95HF, ENABLE); 

	CR95HF_deselect(); 

	SPI_I2S_DeInit (SPI_CR95HF); 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; 
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; 
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; 
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High; 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; 
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; 
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; 
	SPI_InitStructure.SPI_CRCPolynomial = 7; 
	SPI_Init (SPI_CR95HF, & SPI_InitStructure); 
	SPI_Cmd (SPI_CR95HF, ENABLE); 

	SPI_CalculateCRC(SPI_CR95HF, DISABLE);
	SPI_Cmd(SPI_CR95HF, ENABLE);
	while (SPI_I2S_GetFlagStatus(SPI_CR95HF, SPI_I2S_FLAG_TXE) == RESET);
	dummyread = SPI_I2S_ReceiveData(SPI_CR95HF);
}
/*
*	Send command to CR95
*	Input: data and leng of data
*/
void SPI_SendCommand_CR95HF(char *Data,char DataLen)
{
	uint8_t i;
	CR95HF_select();
	for(i=0;i<DataLen;i++)	CR95HF_transfer(Data[i]);      
	CR95HF_deselect();
}

/*
*	Sent byte to cr95
*/
uint8_t CR95HF_transfer(uint8_t b_data)
{
	uint8_t Data = 0;
	uint16_t time_out=0;
	while (SPI_I2S_GetFlagStatus(SPI_CR95HF,SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI_CR95HF, b_data);
	IWDG_ReloadCounter();
	while (SPI_I2S_GetFlagStatus(SPI_CR95HF, SPI_I2S_FLAG_RXNE) == RESET);
	Data = SPI_I2S_ReceiveData(SPI_CR95HF);
	return Data;
}

/**
  * @}
  */
  
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
