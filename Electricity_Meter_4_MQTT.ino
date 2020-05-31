/*Функция подключения*/
void mqtt_Connect()
{
if (flag_MQTT_connected)		{DEBUG_PRINTLN(F("MQTT is connected")); return;}
if (!flag_WiFi_connected)		{DEBUG_PRINTLN(F("WiFi not connected")); return;}
if (!Ping.ping("8.8.8.8", 1))	{DEBUG_PRINTLN(F("MQTT: Internet not connected")); return;}
DEBUG_PRINTLN("MQTT connecting ...");
mqttClient.connect();
}
/**/

/*Обработчик события подключения*/
/*Здесь подписка на топики*/
void mqtt_Connected_Complete(bool sessionPresent)
{
flag_MQTT_connected = true;
DEBUG_PRINTLN(F("Connected to MQTT :)"));
//mqttClient.subscribe("receiv0", 0);
//mqttClient.subscribe("receiv1", 0);
//mqttClient.subscribe("receiv2", 0);
}
/**/

/*Обработчик события обрыва связи*/
void mqtt_Disconnect_Complete(AsyncMqttClientDisconnectReason reason)
{
/*Флаг обрыва связи*/
flag_MQTT_connected = false;
DEBUG_PRINTLN(F("Disconnected from MQTT :("));
}
/**/

/*Обработчик события подписки*/
void mqtt_Subscribe_Complete(uint16_t packetId, uint8_t qos)
{
DEBUG_PRINTLN(String(F("Subscription confirmed, packetId:\t")) + String(packetId) + String("\tqos:\t") + String(qos));
}
/**/

/*Обработчик события отписки*/
void mqtt_Unsubscribe_Complete(uint16_t packetId)
{
DEBUG_PRINTLN(String(F("Unsubscribe confirmed, packetId:\t")) + String(packetId));
}
/**/

/*Обработчик события публикации*/
void mqtt_Publishe_Complete(uint16_t packetId)
{
DEBUG_PRINTLN(String(F("Publication confirmed, packetId:\t")) + String(packetId));
}
/**/

/*Обработчик события приема сообщения*/
/*Здесь обработка принятых сообщений*/
void mqtt_Receiving_Complete(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
String Payload = String (payload).substring(0, len);
if (String(topic).equalsIgnoreCase("receiv0")) mqttClient.publish("send0", 0, false, Payload.c_str());
if (String(topic).equalsIgnoreCase("receiv1")) mqttClient.publish("send1", 0, false, Payload.c_str());
if (String(topic).equalsIgnoreCase("receiv2")) mqttClient.publish("send2", 0, false, Payload.c_str());
DEBUG_PRINTLN(String(xTaskGetTickCount()/1000.0) + "\tpayload:\t" + String(Payload));
}
/**/

/*Функция отправки сообщений MQTT*/
void mqtt_Send()
{
DEBUG_PRINTLN (String(xTaskGetTickCount()/1000.0) + "\tmqtt_Send ()");
if (flag_MQTT_connected)
{
mqttClient.publish("power",			    0, false, String(power, 1).c_str());
mqttClient.publish("energy_month",	0, false, String(energy_month_value, 1).c_str());
mqttClient.publish("voltage",		    0, false, String(voltage, 1).c_str());
mqttClient.publish("current",		    0, false, String(current, 3).c_str());
mqttClient.publish("pf",			      0, false, String(pf, 2).c_str());
mqttClient.publish("frequency",		  0, false, String(frequency, 1).c_str());
mqttClient.publish("energy_global",	0, false, String(energy_current_value, 3).c_str());
mqttClient.publish("rate",			    0, false, String(rate, 1).c_str());
mqttClient.publish("month",			    0, false, String(String(day_before) + " " + String(monthnames[month_before]) + " >>> " + String(energy_before_value, 3) + " kW*h").c_str());
}
}
/**/
