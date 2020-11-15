/*Измерение мгновенных значений напряжения, силы тока, мощности, коэффициента мощности, расхода электроэнергии и денег*/
/*Значения выводятся на экран 240*240, отправляются на смартфон и в облачное хранилище*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#include <PZEM004Tv30.h>
PZEM004Tv30 pzem(&Serial2);

//Библиотека для отключения brownout detector
#include "soc/rtc_cntl_reg.h"

/*Имя платы*/
const char *  board_name PROGMEM = "El_Meter";

/*>-----------< Включение вывода в Serial и (или) Telnet, Bluetooth для отладки >-----------<*/
#define DEBUG 0

#if		  (DEBUG == 1)
#define DEBUG_PRINT(x)      Serial.print(x)
#define DEBUG_PRINTLN(x)    Serial.println(x)
#define DEBUG_WRITE(x, y)   Write(x, y)
#define DEBUG_WRITELN(x, y) Writeln(x, y)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_WRITE(x, y)
#define DEBUG_WRITELN(x, y)
#endif
/*>-----------< Включение вывода в Serial и (или) Telnet, Bluetooth для отладки >-----------<*/

/*Символьные массивы для хранения логина и пароля WiFi в NVS*/
char ssid [30];
char password [30];

#include "Account.h"

/*Account.h содержит регистрационные данные для подключения к серверам MQTT, ThingSpeak, IFTTT*/
/*Очевидно, что свои регистрационные данные я скрыл. Включение файла Account.h нужно заменить на следующие константы*/
/*
//Символьные константы для MQTT
const char * mqtt_host      PROGMEM = "host.cloudmqtt.com";
const char * mqtt_username  PROGMEM = "username";
const char * mqtt_pass      PROGMEM = "pass";
const int  mqtt_port                = 12345;

//Символьные константы для ThingSpeak
const char * thingspeak_write_api_key PROGMEM = "QWERTYUIOPASDFGH";
const char * thingspeak_channel       PROGMEM = "1234567";

//Символьные константы для IFTTT

const char * ifttt_event    PROGMEM = "event";
const char * ifttt_api_key  PROGMEM = "ххххххххххххххххххххххххххххххххххххххххххх";
*/

/*Флаги введения пароля для Terminal, подключения к MQTT, подключения к WiFi, подтверждение приема Now,
таймера малого цикла, таймера большого цикла, разрешения загрузки по OTA, таймера ожидания ответа ThingSpeak, таймера ожидания ответа IFTTT*/
bool flag_Terminal_pass = false, flag_MQTT_connected = false, flag_WiFi_connected = false,
flag_OTA_pass = false, flag_ThSp_time = false, flag_IFTTT_time = false;

/*Библиотеки для OTA, Ping*/
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP32Ping.h>

/*Определения для Bluetooth*/
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

/*Определения для Telnet*/
WiFiServer TelnetServer(59990);
WiFiClient TelnetClient;

/*Определения для MQTT*/
#include <AsyncMqttClient.h>
AsyncMqttClient mqttClient;

/*Определения для NTP*/
#include "time.h"
struct tm timeinfo;

/*Определения для NVS памяти*/
#include <Preferences.h>
Preferences memory;

/*Таймеры*/
#define WatchDog_time		30000		//Время срабатывания сторожевого таймера
#define TACT_time			  300			//Время ожидания ответа сервера ThingSpeak, MQTT, IFTTTT
#define Motion_time			2000

TimerHandle_t timerWatchDog	  = xTimerCreate("timerWatchDog",	  pdMS_TO_TICKS(WatchDog_time),	pdTRUE,	(void*)0,	reinterpret_cast<TimerCallbackFunction_t>(WatchDog));

TimerHandle_t timerMqttSend	  = xTimerCreate("timerMqttSend",	  pdMS_TO_TICKS(TACT_time),		  pdFALSE,	(void*)0,	reinterpret_cast<TimerCallbackFunction_t>(mqtt_Send));
TimerHandle_t timerThSpTime	  = xTimerCreate("timerThSpTime",	  pdMS_TO_TICKS(TACT_time),		  pdFALSE,	(void*)0,	reinterpret_cast<TimerCallbackFunction_t>(ThSpTime));
TimerHandle_t timerIFTTTTime  = xTimerCreate("timerIFTTTTime",	pdMS_TO_TICKS(TACT_time),		  pdFALSE,	(void*)0,	reinterpret_cast<TimerCallbackFunction_t>(IFTTTTime));
TimerHandle_t timerMotion	    = xTimerCreate("timerIFTTTTime",	pdMS_TO_TICKS(Motion_time),		pdFALSE,	(void*)0,	reinterpret_cast<TimerCallbackFunction_t>(Delay_motion));



/*>-----------< Функции >-----------<*/
/*Сторожевой таймер*/
void WatchDog() {DEBUG_PRINTLN(F("\n\n\nThe watchdog timerWDT went off\n\n\n")); ESP.restart();}
/**/

/*Получение времени по NTP каналу*/
void GetTime() {if	(flag_WiFi_connected && Ping.ping("8.8.8.8", 1)) {configTime(10800, 0, "pool.ntp.org"); getLocalTime(&timeinfo);}}
/**/

/*Остановка таймеров перед OTA загрузкой*/
void TimerStop()
{
xTimerStop(timerWatchDog,0);
xTimerStop(timerMqttSend, 0);
xTimerStop(timerThSpTime, 0);
xTimerStop(timerIFTTTTime,0);
xTimerStop(timerMotion,0);
DEBUG_PRINTLN(F(">-----------< TimerStop >-----------<"));
}
/**/
/*>-----------< Функции >-----------<*/

/*>-----------< PZEM >-----------<*/
/*Измерение мгновенных значений напряжения, силы тока, мощности, коэффициента мощности, расхода электроэнергии и денег*/
/*Значения выводятся на экран 240*240, отправляются на смартфон и в облачное хранилище*/

//Подключение TFT ST7789
#define TFT_CS		0	  //Виртуальный пин для компиляции библиотеки
#define TFT_MISO	0	  //Виртуальный пин для компиляции библиотеки
#define TFT_RST		5	  //RES
#define TFT_DC		23	//DC
#define TFT_MOSI	21	//SDA
#define TFT_SCLK	22	//SCL

//Подключение PZEM-004T V3
//PZEM					      ESP32
//+5V					        +3.3V
//Rx					        IO17 (u2TxD)
//Tx					        IO16 (u2RxD)
//GND					        GND

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//Набор цветов
#define BLACK		0x0000
#define WHITE		0xFFFF
#define RED			0xF800
#define GREEN		0x07E0
#define BLUE		0x001F
#define YELLOW	0xFFE0
#define CYAN    0x07FF

bool
flag_update       = true,			  //Начало вывода очередного параметра, при false происходит обработка обработка параметра
flag_motion       = true,			  //Разрешение скроллинга, при false движение строк на экране прекращается
flag_timerenable  = true,	      //Разрешение работы таймера, необходим для исключения лишних, многократных включений таймера
flag_alarm_power  = true,	      //Разрешение отправки оповещения при превышении мгновенной мощности заданного значения, по умолчанию одно оповещение в час
flag_alarm_rate   = true;		    //Разрешение отправки оповещения при превышении накопленной с начала месяца суммы RuR заданного значения, по умолчанию одно оповещение в день

uint8_t
unit_y_offset,				//Смещение строки с единицей измерения по Y
value_y_offset,				//Смещение строки с величиной параметра по Y
parameter =     0,    //Номер обрабатываемого параметра
day_before,           //Номер дня даты ежемесячного фиксирования показаний
month_before,         //Номер месяца даты ежемесячного фиксирования показаний
minute,               //Минуты текущего времени
hour,                 //Часы текущего времени
day,                  //Номер дня текущей даты
month,                //Номер месяца текущей даты
saved_hour,           //Сохраненное для сравнения значение часа
saved_day;            //Сохраненное для сравнения значение дня


uint16_t
width_1,					    //Ширина выводимой строки с единицей измерения
width_2,					    //Ширина выводимой строки с величиной параметра
height_1,					    //Высота выводимой строки с единицей измерения
height_2,					    //Высота выводимой строки с величиной параметра
color,						    //Цвет заливки экрана
color_line;           //Цвет разделительной линии на экране

int16_t 
counter_y =     240,	//Текущая координата Y выводимой картинки
unit_x,						    //Координата X строки с единицей измерения
value_x,					    //Координата X строки с единицей измерения
x_null,						    //Неиспользуемая переменная, необходима для обращения к функции из библиотеки Adafruit_GFX.h
y_null;						    //Неиспользуемая переменная, необходима для обращения к функции из библиотеки Adafruit_GFX.h

float
voltage,					          //Текущее напряжение, В
current,					          //Текущая сила тока, А
power,						          //Текущая мощность, Вт
energy,						          //Накопленный с последнего сброса в PZEM расход электроэнергии, кВт
frequency,					        //Текущая частота, Гц
pf,							            //Текущий коэффициент мощности
energy_main_counter_value,	//Значение расхода добавляемого к расходу PZEM для соответствия реальному счетчику, кВт*ч
energy_before_value,		    //Значение расхода электроэнергии на 00:00 даты ежемесячного фиксирования показаний, кВт*ч
energy_month_value,			    //Текущее значение месячного расхода электроэнергии, energy_month_value = energy_current_value - energy_before_value, кВт*ч
energy_current_value,       //Текущее значение расхода электроэнергии,  energy_current_value = energy_main_counter_value + energy + energy * energy_correction_value, кВт*ч
energy_correction_value,    //Коррекция расхода электроэнергии приведенная к одному кВт*ч, отрицательная, если счетчик спешит, положительная, если отстает
rate_before,                //Расход денег за прошлый месяц, руб.
rate,						            //Расход денег с начала месяца соответсвующий energy_month_value, руб.
price_low =         3.96,		//Стоимость кВт*ч по социальному тарифу, руб./кВт*ч
price_high =        5.53,		//Стоимость кВт*ч сверх социального тарифа, руб./кВт*ч
energy_low_value =  130.0,	//Максимальный расход для социального тарифа, кВт*ч
delay_scroll =      1.0,		//Задержка остановки движения по экрану измеряемого параметра, сек.
alarm_rate,					        //Величина стоимости электроэнергии с начала месяца, после превышения которой отправляется уведомление, руб.
alarm_power;				        //Величина текущей мощности, после превышения которой отправляется уведомление, кВт

char
myStr[10];					        //Вспомогательный массив, используется при округлении действительных чисел и получении значений текущего времени

String
value,						          //Символьное значение величины обрабатываемого параметра
unit,						            //Единица измерения	 обрабатываемого параметра
Status;						          //Переменная для отправки статуса в ThingSpeak

uint8_t unit_text_size      [6] = {9, 9, 9, 9, 9, 9}; //Размеры шрифта единий измерения
uint8_t value_text_size     [6] = {7, 7, 7, 7, 7, 7}; //Размеры шрифта величин параметров

uint16_t unit_text_color    [6] = {WHITE, GREEN, GREEN, CYAN, YELLOW, YELLOW}; //Цвет текста единицы измерения
uint16_t value_text_color   [6] = {WHITE, GREEN, GREEN, CYAN, YELLOW, YELLOW}; //Цвет текста величины параметра

const char
* monthnames[]  = {"", "Янв", "Фев", "Мар", "Апр", "Мая", "Июн", "Июл", "Авг", "Сен", "Окт", "Ноя", "Дек"};

String	 unit_name			    [6] = {"W", "V", "A","Cos", "kWh", "RuR"};     //Соответсвующие параметрам единицы измерения
/*>-----------< PZEM >-----------<*/
