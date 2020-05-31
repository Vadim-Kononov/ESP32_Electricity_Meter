/*Функция подключения к WiFi*/
void WiFiConnect()
{
/*Получение хранящегося логина и пароля WiFi и перевод их в массивы char[]*/
memory.getString("login").toCharArray(ssid, memory.getString("login").length() + 1);
memory.getString("pass").toCharArray(password, memory.getString("pass").length() + 1);

DEBUG_PRINTLN(String(F("Connecting to Wi-Fi...")));
WiFi.mode(WIFI_STA);
WiFi.begin(ssid, password);
WiFi.setHostname(board_name);
WiFi.setAutoConnect(false);
WiFi.setAutoReconnect(false);


/*Инкремент счетчика переподключений*/
memory.putInt("countWifi", memory.getInt("countWifi") + 1);
}
/**/

/*Функция определения WiFi событий*/
void WiFiEvent(WiFiEvent_t event)
{
DEBUG_PRINTLN(String(F("[WiFi-event] event: ")) + String(event));
switch(event)
{
	case SYSTEM_EVENT_STA_GOT_IP:
	flag_WiFi_connected = true;
	DEBUG_PRINTLN(String(F("WiFi connected :)")));
	DEBUG_PRINTLN(WiFi.localIP());
	break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
	flag_WiFi_connected = false;
	DEBUG_PRINTLN(String(F("WiFi connection failure :(")));
	break;
}
}
/**/

/*Функция восстановления подключения к WiFi и MQTT*/
void WiFiReconnect()
{
if (!flag_WiFi_connected) {WiFiConnect();}
mqtt_Connect();
}
/**/
