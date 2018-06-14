/**
  ******************************************************************************
  * �ļ�����: main.c
  * ��    ��: �����硢������
  * ��    ��:
  * ��    ��: V2.3.1
  * ��д����: 2018-05-22
  * ��    ��: stm32�����ѯ����
  ******************************************************************************
  */

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32f10x.h"
#include "usart/bsp_debug_usart.h"
#include "systick/bsp_SysTick.h"
#include "ESP8266/bsp_esp8266.h"
#include "syn6288/bsp_syn6288.h"
#include "spi_flash/bsp_spi_flash.h"
#include "stmflash/stm_flash.h"
#include "sram/bsp_sram.h"
#include "malloc/bsp_malloc.h"
#include "config.h"

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

/* ˽�б��� ------------------------------------------------------------------*/
CityWeather ctweather;
const char g_ssid[]={User_ESP8266_ApSsid};
const char g_pwd[]={User_ESP8266_ApPwd};

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
char aucOut[1024] __attribute__ ((section (".exramtext")));

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

void setESP8266STAMode(char *ssid, char *pwd)
{
	char espPN[] = {"1234"};
	char espTO[] = {"1000"};

	printf("\r\n正在配置 ESP8266 ......\r\n" );

	if(ESP8266_AT_Test())
	{
		printf("AT test OK\r\n");
	}
	printf("\r\n< 1 >\r\n");
	Delay(200);
	if(ESP8266_Net_Mode_Choose(STA))
	{
		printf("ESP8266_Net_Mode_Choose OK\r\n");
	}
	printf("\r\n< 2 >\r\n");
	while(!ESP8266_JoinAP(ssid,pwd));
	printf("\r\n< 3 >\r\n");

#if 1
	ESP8266_Enable_MultipleId(DISABLE);
	while(!ESP8266_Link_Server(enumTCP,User_ESP8266_TcpServer_IP,User_ESP8266_TcpServer_Port,Single_ID_0));
	printf("\r\n< 4 >\r\n");
	while(!ESP8266_UnvarnishSend());
	printf("配置 ESP8266 完毕\r\n");
#else
	ESP8266_Enable_MultipleId(ENABLE);
	while(!ESP8266_StartOrShutServer(ENABLE,espPN,espTO));
	printf("配置 ESP8266 完毕\r\n");
	while(1);
#endif
}

void setESP8266APMode(void)
{
	char espSSID[] = {"esp8266"};
	char espPWD[] = {"esp82666"};
	char espPN[] = {"1234"};
	char espTO[] = {"1000"};
	printf("\r\n正在配置 ESP8266 ......\r\n" );

	if(ESP8266_AT_Test())
	{
		printf("AT test OK\r\n");
	}
	printf("\r\n< 1 >\r\n");
	if(ESP8266_Net_Mode_Choose(AP))
	{
		printf("ESP8266_Net_Mode_Choose OK\r\n");
	}
	printf("\r\n< 2 >\r\n");
	while(!ESP8266_BuildAP(espSSID,espPWD,WPA_WPA2_PSK));
	printf("\r\n< 3 >\r\n");
	ESP8266_Enable_MultipleId(ENABLE);
	while(!ESP8266_StartOrShutServer(ENABLE,espPN,espTO));
	printf("配置 ESP8266 完毕\r\n");
}

void dataToGBK(CityWeather *weather,char * buff)
{
	uint8_t GBK_city[20];
	uint8_t GBK_date[20];
	uint8_t GBK_high[20];
	uint8_t GBK_low[20];
	uint8_t GBK_wind[20];
	int num;

	SwitchToGbk((const unsigned char *)weather->city, strlen(weather->city),GBK_city,&num);
	SwitchToGbk((const unsigned char *)weather->date, strlen(weather->date),GBK_date,&num);
	SwitchToGbk((const unsigned char *)weather->high_tem, strlen(weather->high_tem),GBK_high,&num);
	SwitchToGbk((const unsigned char *)weather->low_tem, strlen(weather->low_tem),GBK_low,&num);
	GBK_high[7] = '\0';
	GBK_low[7] = '\0';
	GBK_wind[0] = (weather->wind_scale)[9];
	GBK_wind[1] = (weather->wind_scale)[10];
	GBK_wind[2] = (weather->wind_scale)[11];
	GBK_wind[3] = '\0';
	sprintf(buff,"6��%s��%s����%s�ȣ�%s�ȣ�����ָ��%s��",GBK_date,GBK_city,GBK_high,GBK_low,GBK_wind);

}


#define FLASHFLAG 0xCCCC
uint16_t g_flashFlag = FLASHFLAG;

/**
  * ��������: ������.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
int main(void)
{
	uint8_t ucStatus;
	char buff[300];
	uint16_t flashFlag;
	uint8_t ssidBuff[50];
	uint8_t pwdBuff[50];

	/* ��ʼ�����Դ��ڣ�115200-N-8-1. */
	DEBUG_USART_Init();
	printf("hello ");
	/* ��ʼ��ϵͳ�δ�ʱ�� */
	SysTick_Init();

	/* ��ʼ���ڴ���� */
	//FSMC_SRAM_Init();
	my_mem_init(SRAMIN);
	my_mem_init(SRAMEX);

	/* ��ʼ��SYN6288��������ģ�� */
	SYN6288_Init();

	/* ��ʼ��ESP8266����ģ�� */
	ESP8266_Init();


	SYN6288_Play("[d][o0][v10][t5]");
	Delay(200);
	SYN6288_Play("ϵͳ��ʼ���ɹ�����������ESP8266");
	Delay(8000);
	/* ��FLASH�ж�ȡ���� */
	STMFLASH_Read(FLASH_flagAddress,&flashFlag,1);
	if(flashFlag == FLASHFLAG)
	{
		SYN6288_Play("���ڶ�ȡ����������˺�����");
		/* ��ȡWIFI�˺����� */
		STMFLASH_Read(FLASH_ssidAddress,(uint16_t *)ssidBuff,25);
		STMFLASH_Read(FLASH_pwdAddress,(uint16_t *)pwdBuff,25);
	}
	else
	{
		SYN6288_Play("����ʹ�����������˺�����");
		setESP8266APMode();
		SYN6288_Play("����APģʽ�ɹ�,�����ӵ�esp8266�������˺�����");
		ESP8266_ReceiveString(ENABLE);
		SYN6288_Play("���ӳɹ�");
		sprintf((char *)ssidBuff,"%s",ESP8266_ReceiveString(ENABLE));
		SYN6288_Play("�˺�Ϊ");
		Delay(3000);
		SYN6288_Play((const char *)(ssidBuff+12));         //12 and 11 is offest address
		sprintf((char *)pwdBuff,"%s",ESP8266_ReceiveString(ENABLE));
		SYN6288_Play("����Ϊ");
		Delay(3000);
		SYN6288_Play((const char *)(pwdBuff+11));
		Delay(5000);
		/* ���ڲ�Flashд���ʼ���� */
		STMFLASH_Write(FLASH_ssidAddress,(uint16_t *)(ssidBuff+12),(strlen((const char *)ssidBuff)-12)/2+1);
		STMFLASH_Write(FLASH_pwdAddress,(uint16_t *)(pwdBuff+11),(strlen((const char *)pwdBuff)-11)/2+1);
		STMFLASH_Write(FLASH_flagAddress,&g_flashFlag,1);
		SYN6288_Play("������ɣ��븴λ����ϵͳ");
		while(1);
	}

	/* ����ESP8266���� */
	setESP8266STAMode((char *)ssidBuff,(char *)pwdBuff);
	SYN6288_Play("ESP8266���óɹ������ڲ�ѯ����");


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

	/*printf("city = %s\r\n",ctweather.city);
	printf("date = %s\r\n",ctweather.date);
	printf("high_tem = %s\r\n",ctweather.high_tem);
	printf("low_tem = %s\r\n",ctweather.low_tem);
	printf("wind_scale = %s\r\n",ctweather.wind_scale);*/
	Delay(8000);

	/* ���������ݱ���תΪGBK */
	dataToGBK(&ctweather,buff);

	/* �������� */
	SYN6288_Play(buff);

	Delay(10000);
	while(1)
	{
		/* �������� */
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
