
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CR95HF_SPI_H
#define __CR95HF_SPI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"

#define SPI_CR95HF                  			SPI2


#define GPIO_SPI_CR95HF              			GPIOB
#define GPIO_Pin_SPI_CR95HF_CS						GPIO_Pin_12
#define GPIO_Pin_SPI_CR95HF_SCK      			GPIO_Pin_13
#define GPIO_Pin_SPI_CR95HF_MISO     			GPIO_Pin_14
#define GPIO_Pin_SPI_CR95HF_MOSI     			GPIO_Pin_15
#define RCC_GPIO_CS_CR95HF   							RCC_APB2Periph_GPIOB
#define RCC_SPI_CR95HF 					    			RCC_APB1Periph_SPI2
 /* - for SPI1 and full-speed APB2: 72MHz/4 */
#define SPI_BaudRateHighSpeed_SPI_CR95HF		SPI_BaudRatePrescaler_4
#define SPI_BaudRateLowSpeed_SPI_CR95HF  		SPI_BaudRatePrescaler_4

#define add_gplx_start 0
#define add_gplx_end 3
#define add_tenlx_start 4	//bao gom 44 byte dau laf ten lx
#define add_tenlx_end   14	
#define add_sdtlx_start 15	//bao gom 16 byte dau laf ten lx
#define add_sdtlx_end   18


/* Port Controls  (Platform dependent) */

#define 	CR95HF_select()					GPIO_ResetBits(GPIO_CS_CR95HF, GPIO_Pin_CS_CR95HF)
#define 	CR95HF_deselect()					GPIO_SetBits(GPIO_CS_CR95HF, GPIO_Pin_CS_CR95HF)
#define	CR95_ON()							GPIO_WriteBit(GPIO_RFID_EN,RFID_EN, (BitAction)(1))

#define	add_gplx_start	0
#define	add_gplx_end	3
#define	add_sdtlx_start	4
#define	add_tenlx_end	14
#define	add_sdtlx_start	15
#define	add_sdtlx_end	18
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#define command_status_RFID 0x02
#define command_write_tag_RFID 0x01

void INIT_SPI2(void);
uint8_t CR95HF_transfer(uint8_t b_data);
void SPI_SendCommand_CR95HF(char *Data,char DataLen);
void CR95HF_interface_speed(void);
void ECH_CR95HF(void);
void sys_CR95HF(void);
void write_secter(uint8_t *buff);
char IDN_CR95HF(void);
char ISO156_CR95HF(void);
void Read_Data_Tag(void);
void TAG_INFO(void);
void support_data_write_cr95hf(uint8_t *buff, uint8_t length);
void Wakeup_CR95HF(void);
void RESET_CR95HF(void);
char wait_polling(char timer_poling);
					 
#endif /* __STM32F10X_IP_DBG_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
