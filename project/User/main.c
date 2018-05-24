/**
  ******************************************************************************
  * �ļ�����: main.c 
  * ��    ��: �����硢������
  * ��    ��: 
  * ��    ��: V2.2.2
  * ��д����: 2018-05-22
  * ��    ��: stm32�����ѯ����
  ******************************************************************************
  */

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32f10x.h"
#include "bsp/usart/bsp_debug_usart.h"
#include "bsp/systick/bsp_SysTick.h"
#include "bsp/ESP8266/bsp_esp8266.h"
#include "bsp/syn6288/bsp_syn6288.h"
#include "bsp/sram/bsp_sram.h"	
#include "bsp/malloc/bsp_malloc.h"	 

#include <string.h>
#include "cjson/cJSON.h"
#include "gzip/zipmem.h"

/* ˽�����Ͷ��� --------------------------------------------------------------*/
typedef struct _CityWeather {
    char *date;
    char *city;
    char *high_tem;
    char *low_tem;
	char *wind_scale;
}CityWeather;

/* ˽�к궨�� ----------------------------------------------------------------*/
#define XINZHI_WEATHER            0  //��֪����
#define JISU_WEATHER              0  //��������
#define CHINA_WEATHER             1  //�й�����


#define User_ESP8266_ApSsid                       "Xiaomi_LAB"              //Ҫ���ӵ��ȵ������
#define User_ESP8266_ApPwd                        "lab141516"           //Ҫ���ӵ��ȵ����Կ

#define User_ESP8266_TcpServer_Port               "80"                 //Ҫ���ӵķ������Ķ˿�

/* ˽�б��� ------------------------------------------------------------------*/
/* ��չ���� ------------------------------------------------------------------*/
extern __IO uint8_t ucTcpClosedFlag;
extern int SwitchToGbk(const unsigned char* pszBufIn, int nBufInLen, unsigned char* pszBufOut, int* pnBufOutLen);

/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/

//������key�滻yourkey

#if JISU_WEATHER

#define User_ESP8266_TcpServer_IP                 "api.jisuapi.com"
char cStr [100] = {"GET http://api.jisuapi.com/weather/query?appkey=yourkey&citycode=101280601\r\n"}; 

void parsingJSON(const char* str, CityWeather* weather)
{
    cJSON* root=cJSON_Parse(str);
    cJSON* results=cJSON_GetObjectItem(root,"results");
	cJSON* block=cJSON_GetArrayItem(results,0);
	
    /* ���� */	
	cJSON* location=cJSON_GetObjectItem(block,"location");
	cJSON* name=cJSON_GetObjectItem(location,"name");
    weather->city = name->valuestring;

	/* ���� */	
	cJSON* daily=cJSON_GetObjectItem(block,"daily");
	cJSON* today=cJSON_GetArrayItem(daily,0);
	cJSON* date=cJSON_GetObjectItem(today,"date");
    weather->date = date->valuestring;
	
	/* ����¶� */
	cJSON* high=cJSON_GetObjectItem(today,"high");
    weather->high_tem = high->valuestring;

	/* ����¶� */
	cJSON* low=cJSON_GetObjectItem(today,"low");
    weather->low_tem = low->valuestring;
	
	/* �����ȼ� */
	cJSON* wind_scale=cJSON_GetObjectItem(today,"wind_scale");
    weather->wind_scale = wind_scale->valuestring;
	
    cJSON_Delete(root);

}

#endif

#if CHINA_WEATHER

#define User_ESP8266_TcpServer_IP                 "wthrcdn.etouch.cn"
char cStr [80] = {"GET http://wthrcdn.etouch.cn/weather_mini?citykey=101250201\r\n"};
char aucOut[400];

void parsingJSON(const char* str, CityWeather* weather)
{
	cJSON* root=cJSON_Parse(str);
	cJSON* data=cJSON_GetObjectItem(root,"data");
	
	/* ���� */	
	cJSON* city=cJSON_GetObjectItem(data,"city");
    weather->city = city->valuestring;
	
	/* ���� */ 
	cJSON* forecast=cJSON_GetObjectItem(data,"forecast");
	cJSON* today=cJSON_GetArrayItem(forecast,0);
	cJSON* date=cJSON_GetObjectItem(today,"date");
    weather->date = date->valuestring;
		
	/* ����¶� */
	cJSON* high=cJSON_GetObjectItem(today,"high");
    weather->high_tem = high->valuestring;

	/* ����¶� */
	cJSON* low=cJSON_GetObjectItem(today,"low");
    weather->low_tem = low->valuestring;
	
	/* �����ȼ� */
	cJSON* fengli=cJSON_GetObjectItem(today,"fengli");
    weather->wind_scale = fengli->valuestring;
	
	cJSON_Delete(root);
}

#endif

#if XINZHI_WEATHER

#define User_ESP8266_TcpServer_IP                 "api.seniverse.com"     
char cStr [131] = {"GET https://api.seniverse.com/v3/weather/daily.json?key=yourkey&location=xiangtan&language=zh-Hans&unit=c&start=0&days=5\r\n"};

void parsingJSON(const char* str, CityWeather* weather)
{
	cJSON* root=cJSON_Parse(str);
    cJSON* result=cJSON_GetObjectItem(root,"result");
	
	/* ���� */	
	cJSON* city=cJSON_GetObjectItem(result,"city");
    weather->city = city->valuestring;
	    
	cJSON_Delete(root);
}

#endif

/**
  * ��������: ������.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
int main(void)
{ 
	uint8_t ucStatus;  
	CityWeather ctweather;
	
	/* ���Դ��ڳ�ʼ�����ã�115200-N-8-1.ʹ�ܴ��ڷ��ͺͽ��� */
	DEBUG_USART_Init();  
	
	FSMC_SRAM_Init();			//��ʼ���ⲿSRAM  
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);		//��ʼ���ⲿ�ڴ��
	
	SysTick_Init();

	SYN6288_Init();
		
	ESP8266_Init();
	SYN6288_Play("[v8]ϵͳ��ʼ���ɹ�����������ESP8266");
	printf("\r\n正在配置 ESP8266 ......\r\n" );

	if(ESP8266_AT_Test())
	{
		printf("AT test OK\r\n");
	}
	printf("\r\n< 1 >\r\n");
	if(ESP8266_Net_Mode_Choose(STA))
	{
		printf("ESP8266_Net_Mode_Choose OK\r\n");
	}  
	printf("\r\n< 2 >\r\n");
	while(!ESP8266_JoinAP(User_ESP8266_ApSsid,User_ESP8266_ApPwd));		
	printf("\r\n< 3 >\r\n");
	ESP8266_Enable_MultipleId(DISABLE);	
	while(!ESP8266_Link_Server(enumTCP,User_ESP8266_TcpServer_IP,User_ESP8266_TcpServer_Port,Single_ID_0));	
	printf("\r\n< 4 >\r\n");
	while(!ESP8266_UnvarnishSend());	
	printf("配置 ESP8266 完毕\r\n");

	SYN6288_Play("[v8]ESP8266���óɹ������ڲ�ѯ����");
	/* ����ѭ�� */
	while (1)
	{
		/* ����GET���� */
		while (!ESP8266_SendString(ENABLE,cStr,131,Single_ID_0)); 
		/* ��ȡ�������ݰ� */
		char *httpData = ESP8266_ReceiveString(ENABLE);
		
		
	#if CHINA_WEATHER	  		
		/* gzip��ѹԭʼ���� */
		unzipmem(httpData, strEsp8266_Fram_Record .InfBit .FramLength, aucOut); 
		/* ���JSON���ݻ�ȡ�������� */
		parsingJSON((const char*)aucOut, &ctweather);
	#else
		/* ���JSON���ݻ�ȡ�������� */
		parsingJSON((const char*)httpData, &ctweather);
	#endif
		
		printf("city = %s\r\n",ctweather.city);
		printf("date = %s\r\n",ctweather.date);
		printf("high_tem = %s\r\n",ctweather.high_tem);
		printf("low_tem = %s\r\n",ctweather.low_tem);
		printf("wind_scale = %s\r\n",ctweather.wind_scale);
		
		Delay(8000);
		
		uint8_t GBK_city[20];
		uint8_t GBK_date[20];
		uint8_t GBK_high[20];
		uint8_t GBK_low[20];
		uint8_t GBK_wind[20];
		int num;
		char strl[300];
		SwitchToGbk((const unsigned char *)ctweather.city, strlen(ctweather.city),GBK_city,&num);
		SwitchToGbk((const unsigned char *)ctweather.date, strlen(ctweather.date),GBK_date,&num);
		SwitchToGbk((const unsigned char *)ctweather.high_tem, strlen(ctweather.high_tem),GBK_high,&num);
		SwitchToGbk((const unsigned char *)ctweather.low_tem, strlen(ctweather.low_tem),GBK_low,&num);
		SwitchToGbk((const unsigned char *)ctweather.wind_scale, strlen(ctweather.wind_scale),GBK_wind,&num);
		sprintf(strl,"[v8]%s��%s��������¶�%s������¶�%s������ָ��%s",GBK_date,GBK_city,GBK_high,GBK_low,GBK_wind);
		SYN6288_Play(strl);
		
		while(1);
		
		#pragma diag_suppress 111
		if(ucTcpClosedFlag)                                             //����Ƿ�ʧȥ����
		{
			ESP8266_ExitUnvarnishSend();                                    //�˳�͸��ģʽ			
			do ucStatus = ESP8266_Get_LinkStatus();                         //��ȡ����״̬
			while(!ucStatus);			
			if(ucStatus==4)                                             //ȷ��ʧȥ���Ӻ�����
			{
				printf("正在重连热点和服务器 ......\r\n");				
				while(!ESP8266_JoinAP(User_ESP8266_ApSsid,User_ESP8266_ApPwd));				
				while(!ESP8266_Link_Server(enumTCP,User_ESP8266_TcpServer_IP,User_ESP8266_TcpServer_Port,Single_ID_0));				
				printf("重连热点和服务器成功!!!\r\n");
			}			
			while(!ESP8266_UnvarnishSend());					
		}
	}
}

/******************* (C) COPYRIGHT 2014-2019 ѧ������ʵ���� *****END OF FILE****/
