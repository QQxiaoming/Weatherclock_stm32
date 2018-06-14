/**
  ******************************************************************************
  * �ļ�����: bsp_syn6288.c 
  * ��    ��: ������
  * ��    ��: V1.0
  * ��д����: 2018-05-24
  * ��    ��: syn6288Ӳ������
  ******************************************************************************
  */
/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "syn6288/bsp_syn6288.h"
#include <string.h>

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/

void SYN6288_Init(void)
{
	SYN6288_USART_Config();
}

void SYN6288_Play(const char *str)
{
	uint8_t headOfFrame[5];  
	uint8_t ecc  = 0;  			//����У���ֽ�
	uint8_t length;
	uint32_t i=0; 
	
	length =strlen(str); 			//��Ҫ�����ı��ĳ���
	
	headOfFrame[0] = 0xFD ; 			//����֡ͷFD
	headOfFrame[1] = 0x00 ; 			//�������������ȵĸ��ֽ�
	headOfFrame[2] = length + 3; 		//�������������ȵĵ��ֽ�
	headOfFrame[3] = 0x01 ; 			//���������֣��ϳɲ�������
	headOfFrame[4] = 0x01 ;
	
	for(i = 0; i<5; i++)   				//���η��͹���õ�5��֡ͷ�ֽ�
	{  
		ecc=ecc^(headOfFrame[i]); 		//�Է��͵��ֽڽ������У��	
		SYN6288_Usart_SendByte(headOfFrame[i]);
	}

	for(i = 0; i<length; i++)   		//���η��ʹ��ϳɵ��ı�����
	{  
		ecc=ecc^(str[i]); 				//�Է��͵��ֽڽ������У��	
		SYN6288_Usart_SendByte(str[i]);		
	}

	SYN6288_Usart_SendByte(ecc);
}

/**
  * ��������: ����NVIC���趨USART�����ж����ȼ�.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static void SYN6288_USART_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	 /* Ƕ�������жϿ�������ѡ�� */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
   /* ����USARTΪ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = SYN6288_USART_IRQn;	 
	 /* �������ȼ�Ϊ0 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	 /* �����ȼ�Ϊ1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	 /* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 /* ��ʼ������NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * ��������: ���ڲ�������.
  * �������: ��
  * �� �� ֵ: ��
  */
void SYN6288_USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
  
    SYN6288_USART_NVIC_Configuration();
	
    SYN6288_USART_ClockCmd(SYN6288_USART_CLK,ENABLE);
    SYN6288_USART_GPIO_ClockCmd(SYN6288_USART_TX_CLK | SYN6288_USART_RX_CLK | RCC_APB2Periph_AFIO,ENABLE);
  
	/* ����USART����GPIO��ʼ�� */
	GPIO_InitStructure.GPIO_Pin =  SYN6288_USART_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SYN6288_USART_TX_PORT, &GPIO_InitStructure);    
  
	GPIO_InitStructure.GPIO_Pin = SYN6288_USART_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SYN6288_USART_RX_PORT, &GPIO_InitStructure);	
			
	/* USART������������ */
	USART_InitStructure.USART_BaudRate = SYN6288_USART_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(SYN6288_USART, &USART_InitStructure);
  
  //USART_ITConfig(SYN6288_USART, USART_IT_RXNE, ENABLE);	
  
	USART_Cmd(SYN6288_USART, ENABLE);
	USART_ClearFlag(SYN6288_USART, USART_FLAG_TC|USART_FLAG_TXE|USART_FLAG_RXNE);
}

/**
  * ��������: ���ڷ���һ���ֽ����� 
  * �������: ch���������ַ�
  * �� �� ֵ: ��
  * ˵    ������
  */
void SYN6288_Usart_SendByte(uint8_t ch)
{
  /* ����һ���ֽ����ݵ�USART1 */
  USART_SendData(SYN6288_USART,ch);

  /* �ȴ�������� */
  while (USART_GetFlagStatus(SYN6288_USART, USART_FLAG_TXE) == RESET);	
}

/**
  * ��������: ���ڷ���ָ�����ȵ��ַ���
  * �������: str���������ַ���������
  *           strlen:ָ���ַ�������
  * �� �� ֵ: ��
  * ˵    ������
  */
void SYN6288_Usart_SendStr_length(uint8_t *str,uint32_t strlen)
{
	unsigned int k=0;
	do 
	{
		SYN6288_Usart_SendByte(*(str + k));
		k++;
	} while(k < strlen);
}

/**
  * ��������: ���ڷ����ַ�����ֱ�������ַ���������
  * �������: str���������ַ���������
  * �� �� ֵ: ��
  * ˵    ������
  */
void SYN6288_Usart_SendString(uint8_t *str)
{
	unsigned int k=0;
	do 
	{
		SYN6288_Usart_SendByte(*(str + k));
		k++;
	} while(*(str + k)!='\0');
}

/******************* (C) COPYRIGHT 2014-2019 ѧ������ʵ���� *****END OF FILE****/
