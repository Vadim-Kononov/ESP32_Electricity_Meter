/*Отправка на сервер IFTTT методом POST способом JSON*/
bool IFTTTSend (String event, String value1, String value2, String value3)
{
if (!flag_WiFi_connected)		  {DEBUG_PRINTLN(String (F("flag_WiFi_connected = ")) + String (flag_WiFi_connected));	return false;}
if (!Ping.ping("8.8.8.8", 1))	{DEBUG_PRINTLN(String (F("ping 8.8.8.8 = ")) + String (Ping.ping("8.8.8.8", 1)));		return false;}

String data_to_send = 	"";
data_to_send += 		F("{\"value1\":\"");
data_to_send += 		value1;
data_to_send += 		F("\",\"value2\":\"");
data_to_send += 		value2;
data_to_send += 		F("\",\"value3\":\"");
data_to_send += 		value3;
data_to_send += 		F("\"}");

char server[] = "maker.ifttt.com";

WiFiClient iftttClient;
iftttClient.stop();


if (iftttClient.connect(server, 80))
{
DEBUG_PRINTLN(data_to_send);
iftttClient.println
(
	String(F("POST /trigger/"))
+ 	event
+ 	String(F("/with/key/"))
+ 	ifttt_api_key
+ 	String(F(" HTTP/1.1\n"))
+ 	F("Host: maker.ifttt.com\nConnection: close\nContent-Type: application/json\nContent-Length: ")
+ 	String(data_to_send.length())
+ 	"\n\n"
+ 	String(data_to_send)
);
}
else
{
DEBUG_PRINTLN(F("Failed to connect to IFTTT"));
}

/*Ответ сервера*/
flag_IFTTT_time = false;
xTimerChangePeriod(timerIFTTTTime,  pdMS_TO_TICKS(TACT_time), 0);
while (iftttClient.available() == 0 && !flag_IFTTT_time) {};
if (flag_IFTTT_time) {iftttClient.stop(); return false;}

iftttClient.parseFloat();
String resp = String(iftttClient.parseInt());
DEBUG_PRINTLN(String(F("Response code: ")) + resp + "\n");
if (resp.equalsIgnoreCase("200")) return true; else return false;
}

/*Срабатывания таймера ожидания ответа сервера*/
void IFTTTTime ()
{
flag_IFTTT_time = true;
}

/* POST JSON Format
iftttClient.println("POST /trigger/" + event + "/with/key/" + ifttt_api_key + " HTTP/1.1");
iftttClient.println(F("Host: maker.ifttt.com"));
iftttClient.println(F("Connection: close"));
iftttClient.println(F("Content-Type: application/json"));
iftttClient.print(F("Content-Length: "));
iftttClient.println(data_to_send.length());
iftttClient.println();
iftttClient.print(data_to_send);
*/
/**/
