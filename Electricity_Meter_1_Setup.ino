void setup()
{
//Отключение brownout detector
WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

#if   (DEBUG == 1)
Serial.begin(115200);
#endif

/*Инициализация NVS памяти*/
memory.begin("memory", false);

/*Первоначальное (первое для платы) сохранение логина и пароля WiFi закомментировано*/
//memory.putString("login", "One"); memory.putString("pass", "73737373");

/*Назначение функции WiFiEvent*/
WiFi.onEvent(WiFiEvent);

/*Подключение к WiFi*/
WiFiConnect();

/*Инициализация Bluetooth*/
SerialBT.begin(board_name);

/*Инициализация Telnet*/
TelnetServer.begin();
TelnetServer.setNoDelay(true);

/*Назначение функций и параметров OTA*/
ArduinoOTA.setHostname(board_name);
ArduinoOTA.onStart(TimerStop);

/*Назначение функций и параметров MQTT*/
mqttClient.onConnect(mqtt_Connected_Complete);
mqttClient.onDisconnect(mqtt_Disconnect_Complete);
mqttClient.onSubscribe(mqtt_Subscribe_Complete);
mqttClient.onUnsubscribe(mqtt_Unsubscribe_Complete);
mqttClient.onMessage(mqtt_Receiving_Complete);
mqttClient.onPublish(mqtt_Publishe_Complete);
mqttClient.setServer(mqtt_host, mqtt_port);
mqttClient.setCredentials(mqtt_username, mqtt_pass);
mqttClient.setClientId(board_name);

/*Получение времени с NTP сервера*/
GetTime();

/*>-----------< PZEM >-----------<*/
//Инициализация экрана
SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
tft.init(240, 240, SPI_MODE3);
tft.setRotation(3);
tft.setTextWrap(0);
//Определение и установка цвета заливки
color = tft.color565(0, 0, 10);
tft.fillScreen(color);
//Загрузка сохраненных в NVS значений переменных
energy_correction_value 	= memory.getFloat("correction", 0.0);
energy_before_value 		  = memory.getFloat("before", 0.0);

day_before 					      = memory.getInt("day", day_before);
month_before 				      = memory.getInt("month", month_before);
delay_scroll 				      = memory.getFloat("delay", delay_scroll);

price_low 					      = memory.getFloat("price1", price_low);
price_high 					      = memory.getFloat("price2", price_high);
energy_low_value 			    = memory.getFloat("low_value", energy_low_value);

alarm_power 				      = memory.getFloat("alarm_power", 1000.0);
alarm_rate 					      = memory.getFloat("alarm_rate", 10000.0);
//Стирание не используемых ключей
//memory.remove("");

/*>-----------< PZEM >-----------<*/

/*Запуск сторожевого таймера*/
xTimerStart(timerWatchDog, 0);

/*Инкремент счетчика перезагрузок*/
memory.putInt("countReset", memory.getInt("countReset") + 1);

/*Отправка IFTTT оповещения о перезагрузке*/
IFTTTSend (String(ifttt_event), String(board_name) + " " + String(F("Reloading ")), String("Res:") + String (memory.getInt("countReset")), String(" WiFi:") + String (memory.getInt("countWifi")));
}
