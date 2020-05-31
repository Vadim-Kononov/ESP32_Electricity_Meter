/*Вывод в терминалы Serial, WiFiSerial, BTSerial*/
void Write(String string, uint8_t terminal)
{
/*B001 Serial | B010 WiFi | B100 BT*/
if bitRead(terminal, 0) DEBUG_PRINT(string);
if bitRead(terminal, 1) TelnetClient.print(string);
if bitRead(terminal, 2) SerialBT.print(string);
}
/**/

/*Вывод в терминалы Serial, WiFiSerial, BTSerial*/
void Writeln(String string, uint8_t terminal)
{
/*B001 Serial | B010 WiFi | B100 BT*/
if bitRead(terminal, 0) DEBUG_PRINTLN(string);
if bitRead(terminal, 1) TelnetClient.println(string);
if bitRead(terminal, 2) SerialBT.println(string);
}
/**/

/*Обработка строки принятой через Telnet или Bluetooth*/
void Terminal(String string, uint8_t terminal)
{
String string_1 = "", string_2 = "", string_3 = "", string_4 = "";

/*Парсинг входной строки на 4 элемента по "/"*/
if (string.indexOf("/") >=0) {string_1 = string.substring(0, string.indexOf("/")); string.remove(0, string.indexOf("/") + 1);} else {string.trim(); string_1 = string; string.remove(0, string.length());}
if (string.indexOf("/") >=0) {string_2 = string.substring(0, string.indexOf("/")); string.remove(0, string.indexOf("/") + 1);} else {string.trim(); string_2 = string; string.remove(0, string.length());}
if (string.indexOf("/") >=0) {string_3 = string.substring(0, string.indexOf("/")); string.remove(0, string.indexOf("/") + 1);} else {string.trim(); string_3 = string; string.remove(0, string.length());}
if (string.indexOf("/") >=0) {string_4 = string.substring(0, string.indexOf("/")); string.remove(0, string.indexOf("/") + 1);} else {string.trim(); string_4 = string; string.remove(0, string.length());}

/* Если был введен Password*/
if (flag_Terminal_pass)
{

/* "Password". Отключение пароля, если он включен. Режим триггера*/
if (string_1.equalsIgnoreCase("cbhLtpjr")) {flag_Terminal_pass = false; flag_OTA_pass = false; Writeln ("< ", terminal); return;}

/* "Login". Запись в память логина и пароля WiFi*/
else if (string_1.equalsIgnoreCase("Login"))
{
if (!string_2.equalsIgnoreCase("") && !string_3.equalsIgnoreCase("")) {memory.putString("login", string_2); memory.putString("pass", string_3);}
Writeln("> Login: " + memory.getString("login") + " | Pass: " + memory.getString("pass"), terminal);
}

/* "Reconnect". Переподключение WiFi по логину и паролю хранящимися в памяти*/
else if (string_1.equalsIgnoreCase("Reconnect"))
{
Writeln ("> Reconnect WiFi to: " + memory.getString("login"), terminal);
WiFi.disconnect(); WiFiConnect();
}

/* "Reset". Перезагрузка модуля*/
else if (string_1.equalsIgnoreCase("Reset")) {Writeln("> Reset", terminal); delay (250); ESP.restart();}

/* "Count". Вывод счетчиков перезагрузок и реконнектов*/
else if (string_1.equalsIgnoreCase("Count")) Writeln("> Count: Reset " + String(memory.getInt("countReset")) + " | " + "Wifi " + String(memory.getInt("countWifi")), terminal);

/* "CountRes". Сброс счетчиков перезагрузок и реконнектов*/
else if (string_1.equalsIgnoreCase("CountRes")) {memory.putInt("countReset", 0); memory.putInt("countWifi", 0); Writeln("> Count: Reset " + String(memory.getInt("countReset")) + " | " + "Wifi " + String(memory.getInt("countWifi")), terminal);}

/* "WiFi". Вывод текущих параметров WiFi*/
else if (string_1.equalsIgnoreCase("WiFi")) {Writeln("> " + String(WiFi.SSID()) + " : " + WiFi.channel() + " (" + String(WiFi.RSSI()) + ") " + WiFi.localIP().toString(), terminal);}

/* "Scan". Сканирование WiFi*/
else if (string_1.equalsIgnoreCase("Scan"))
{
Writeln("> Scan: ", terminal);
int n = WiFi.scanNetworks();
String str = "";
if (n == 0) {Writeln (">Сети не найдены", terminal);}
else {for (int i = 0; i < n; ++i) {str += String(i + 1); str += ". "; str += WiFi.SSID(i); str += " : "; str +=  WiFi.channel(i); str += " (";  str += WiFi.RSSI(i); str += ") \n";} Write(str, terminal);}
}

/* "Time". Вывод текущего времени, обращение к элементам структуры: timeinfo.tm_sec|timeinfo.tm_min|timeinfo.tm_hour и т.д., тип - int*/
else if (string_1.equalsIgnoreCase("Time")) {char time_str[50]; GetTime(); strftime(time_str, 50, "%A %d %b %Y %H:%M:%S", &timeinfo); Writeln("> " + String(time_str), terminal);}

/* "Mem". Вывод количества свободной памяти*/
else if (string_1.equalsIgnoreCase("Mem")) {Writeln ("> Free memory: " + String(ESP.getFreeHeap()), terminal);}

/* "OTA". Разрешение загрузки по воздуху*/
else if (string_1.equalsIgnoreCase("OTA")) {ArduinoOTA.begin(); flag_OTA_pass = true; Writeln ("> OTA On: " + String(ESP.getFreeSketchSpace()), terminal);}

/* "PZEM". Настройка параметров счетчика*/
else if (string_1.equalsIgnoreCase("PZEM") && !string_2.equalsIgnoreCase(""))
{
if (string_2.equalsIgnoreCase("Now"))
{
if (!string_3.equalsIgnoreCase("")) {pzem.resetEnergy(); energy_correction_value = string_3.toFloat(); memory.putFloat("correction", energy_correction_value);}
Writeln ("> PZEM: Now value " + String(energy_correction_value, 3)  + String(" kWh"), terminal);
}

else if (string_2.equalsIgnoreCase("Before"))
{
if (!string_3.equalsIgnoreCase("")) {energy_before_value = string_3.toFloat(); memory.putFloat("before", energy_before_value);}
Writeln ("> PZEM: Before value " + String(energy_before_value, 3)  + String(" kWh"), terminal);
}

else if (string_2.equalsIgnoreCase("Date"))
{
if (!string_3.equalsIgnoreCase("") && !string_4.equalsIgnoreCase("")) {day_before = string_3.toInt(); month_before = string_4.toInt(); memory.putInt("day", day_before); memory.putInt("month", month_before);}
Writeln ("> PZEM: Before date " + String(day_before) + " " + String(monthnames[month_before]), terminal);
}

else if (string_2.equalsIgnoreCase("Delay"))
{
if (!string_3.equalsIgnoreCase("")) {delay_scroll = string_3.toFloat(); memory.putFloat("delay", delay_scroll);}
Writeln ("> PZEM: Delay " + String(delay_scroll) + String(" sec"), terminal);
}

else if (string_2.equalsIgnoreCase("PriceLow"))
{
if (!string_3.equalsIgnoreCase("")) {price_low = string_3.toFloat(); memory.putFloat("price1", price_low);}
Writeln ("> PZEM: Price low " + String(price_low, 2) + String(" RUR"), terminal);
}

else if (string_2.equalsIgnoreCase("PriceHigh"))
{
if (!string_3.equalsIgnoreCase("")) {price_high = string_3.toFloat(); memory.putFloat("price2", price_high);}
Writeln ("> PZEM: Price high " + String(price_high, 2) + String(" RUR"), terminal);
}

else if (string_2.equalsIgnoreCase("LowLimit"))
{
if (!string_3.equalsIgnoreCase("")) {energy_low_value = string_3.toFloat(); memory.putFloat("low_value", energy_low_value);}
Writeln ("> PZEM: Low limit " + String(energy_low_value, 1) + String(" kWh"), terminal);
}

else if (string_2.equalsIgnoreCase("AlarmPower"))
{
if (!string_3.equalsIgnoreCase("")) {alarm_power = string_3.toFloat(); memory.putFloat("alarm_power", alarm_power); flag_alarm_power = true;}
Writeln ("> PZEM: Alarm Power " + String(alarm_power, 3) + String(" kWh"), terminal);
}

else if (string_2.equalsIgnoreCase("AlarmRate"))
{
if (!string_3.equalsIgnoreCase("")) {alarm_rate = string_3.toFloat(); memory.putFloat("alarm_rate", alarm_rate); flag_alarm_rate = true;}
Writeln ("> PZEM: Alarm Rate " + String(alarm_rate, 1) + String(" RUR"), terminal);
}
else Writeln ("> PZEM: Invalid PZEM string", terminal);

}
/* "Help".*/
else if (string_1.equalsIgnoreCase("?") || string_1.equalsIgnoreCase("Help"))
{Writeln (">" +
String("\n| Login/<SSID>/<Password> |") +
String("\n| Scan | WiFi | Reconnect | Reset|") +
String("\n| Count| CountRes   |") +
String("\n| Time | Mem  | OTA |") +
String("\n| PZEM/Now/<kWh> |") +
String("\n| PZEM/Before/<kWh> |") +
String("\n| PZEM/Date/<day>/<month>|") +
String("\n| PZEM/Delay/<sec> |") +
String("\n| PZEM/PriceLow/<RUR>  |") +
String("\n| PZEM/PriceHigh/<RUR> |") +
String("\n| PZEM/LowLimit/<kWh>  |") +
String("\n| PZEM/AlarmPower/<kWh>|") +
String("\n| PZEM/AlarmRate/<RUR> |") +
String("\n| ? | Help |") +
String("\n>")
, terminal);}

/* Все остальное*/
else
{
Writeln ("1>/" + string_1, terminal);
Writeln ("2>/" + string_2, terminal);
Writeln ("3>/" + string_3, terminal);
Writeln ("4>/" + string_4, terminal);
Writeln (">>/" + string, terminal);
}
}

/* Обработка пароля после перезагрузки*/
if (!flag_Terminal_pass) {if (!string_1.compareTo("cbhLtpjr")) {flag_Terminal_pass = true; Writeln ("> ", terminal);} else Writeln ("Password > ", terminal);}
}
