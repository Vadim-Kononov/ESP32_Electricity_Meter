/*Отправка на сервер ThingSpeakSend методом POST способом CSV*/
bool ThingSpeakSend ()
{
if (!flag_WiFi_connected)		  {DEBUG_PRINTLN(String (F("flag_WiFi_connected = ")) + String (flag_WiFi_connected));	return false;}
if (!Ping.ping("8.8.8.8", 1))	{DEBUG_PRINTLN(String (F("ping 8.8.8.8 = ")) + String (Ping.ping("8.8.8.8", 1)));		return false;}

String data_to_send = "";
data_to_send += F("write_api_key=");
data_to_send += String(thingspeak_write_api_key);
data_to_send += F("&time_format=relative&updates=30,");
data_to_send += String(power, 3);
data_to_send += F(",");
data_to_send += String(voltage, 1);
data_to_send += F(",");
data_to_send += String(current, 3);
data_to_send += F(",");
data_to_send += String(pf, 2);
data_to_send += F(",");
data_to_send += String(frequency, 1);
data_to_send += F(",");
data_to_send += String(energy_current_value, 3);
data_to_send += F(",");
data_to_send += String(energy_current_value - energy_before_value, 3);
data_to_send += F(",");
data_to_send += String(rate, 1);
data_to_send += F(",");
data_to_send += F("0,");
data_to_send += F("0,");
data_to_send += F("0,");
data_to_send += (Status);

char server[] = "api.thingspeak.com";

WiFiClient thingspeakClient;
thingspeakClient.stop();


if (thingspeakClient.connect(server, 80))
{
DEBUG_PRINTLN(data_to_send);
thingspeakClient.println
(
String(F("POST /channels/"))
+ String(thingspeak_channel)
+ String(F("/bulk_update.csv HTTP/1.1\nHost: api.thingspeak.com\nConnection: close\nUser-Agent: ESP32Exch\nContent-Type: application/x-www-form-urlencoded\nContent-Length: "))
+ String(data_to_send.length())
+ "\n\n"
+ String(data_to_send)
);
}
else
{
DEBUG_PRINTLN(F("Failed to connect to ThingSpeak"));
}
/*Ответ сервера*/
flag_ThSp_time = false;
xTimerChangePeriod(timerThSpTime,  pdMS_TO_TICKS(TACT_time), 0);
while (thingspeakClient.available() == 0 && !flag_ThSp_time) {};
if (flag_ThSp_time) {thingspeakClient.stop(); return false;}

thingspeakClient.parseFloat();
String resp = String(thingspeakClient.parseInt());
DEBUG_PRINTLN(String(F("Response code: ")) + resp + "\n");
if (resp.equalsIgnoreCase("202")) return true; else return false;
}
/**/
/*Срабатывания таймера ожидания ответа сервера*/
void ThSpTime ()
{
flag_ThSp_time = true;
}
/**/
/* POST CSV Format data_to_send = "write_api_key=THINGSPEAK_WRITE_API_KEY&time_format=relative&updates=30,1,,3,,5,,7,,50,50,50,Status";
thingspeakClient.println("POST /channels/" + String(thingspeak_channel) + "/bulk_update.csv HTTP/1.1");
thingspeakClient.println("Host: api.thingspeak.com");
thingspeakClient.println("Connection: close");
thingspeakClient.println("User-Agent: ESP32Exch");
thingspeakClient.println("Content-Type: application/x-www-form-urlencoded");
thingspeakClient.print("Content-Length: ");
thingspeakClient.println(data_to_send.length());
thingspeakClient.println();
thingspeakClient.print(data_to_send);
*/

/* POST Txt Format data_to_send = "&field1=1&field2=null&field3=3&field4=null&field5=5&field6=null&field7=7&field8=null&latitude=50&longitude=50&elevation=50&status=Status\r\n\r\n";
thingspeakClient.println("POST /update HTTP/1.1");
thingspeakClient.println("Host: api.thingspeak.com");
thingspeakClient.println("Connection: close");
thingspeakClient.println("User-Agent: ESP32Exch");
thingspeakClient.println("X-THINGSPEAKAPIKEY: " + write_api_key);
thingspeakClient.println("Content-Type: application/x-www-form-urlencoded");
thingspeakClient.print("Content-Length: ");
thingspeakClient.println(data_to_send.length());
thingspeakClient.println();
thingspeakClient.print(data_to_send);
*/
/**/
