void loop()
{
if (flag_OTA_pass) ArduinoOTA.handle();



/*Проверка подключения клиентов Telnet*/
if (TelnetServer.hasClient())
{
if (!TelnetClient || !TelnetClient.connected())
{
if(TelnetClient) TelnetClient.stop();
TelnetClient = TelnetServer.available();
TelnetClient.println("My IP:\t" + WiFi.localIP().toString() + "\t|\tYour IP:\t" + TelnetClient.remoteIP().toString());
flag_OTA_pass = false;
}
else TelnetServer.available().stop();
}
/*Получение строки из Telnet, при ее наличии. Отправка строки в Terminal для обработки*/
if (TelnetClient && TelnetClient.connected() && TelnetClient.available()) {while(TelnetClient.available()) Terminal(TelnetClient.readStringUntil('\n'), B010);}



/*Получение строки из Bluetooth, при ее наличии. Отправка строки в Terminal для обработки*/
if (SerialBT.available()) Terminal(SerialBT.readStringUntil('\n'), B100);



/*>-----------< PZEM >-----------<*/
//Вход в обновление и вывод очередного параметра
if (flag_update)
{
//Сброс флага разрешающего обновление
flag_update = false;
//Проверка номера выводимого параметра
if (parameter>=6) parameter = 0;

//Для каждого из параметров
switch (parameter)
{
/*Мощность*/
case 0:
//Предварительная запись неопределенного значения
value = "NaN";
//Проверка нахождения параметра в области допустимых значений
if (power >= 0.0 && power < 10000.0)
{
//Пять символов на ширину экрана, первое условие выводит Ватты
if		(power < 1000.0)  {value =	String(power, 1); unit_name [parameter] = "W"; unit_text_color [parameter] = WHITE; value_text_color [parameter] = WHITE;}
//Второе условие выводит киловатты, с соответствующим изменением цвета текста
else if (power < 10000.0) {dtostrf(round(power*1.0)/1000.0, 5, 3, myStr); value = String(myStr); unit_name [parameter] = "kW"; unit_text_color [parameter] = RED; value_text_color [parameter] = RED;}
}
//Установка размера, цвета текста и вычисление смещений строк по X и Y
Format ();
break;

/*Напряжение*/
case 1:
value = "NaN";
if (voltage >= 100.0 && voltage <= 400.0) value = String(voltage, 1);
Format ();
break;

/*Сила тока*/
case 2:
value = "NaN";
if (current >= 0.0 && current < 100.0)
{
if		(current < 10.0)  value =  String(current, 3);
else if (current < 100.0) value =  String(current, 2);
}
Format ();
break;

/*Косинус φ*/
case 3:
value = "NaN";
if (pf >= 0.0 && pf < 10.0) value = pf;
Format ();
break;

/*Расход электроэнергии c начала месяца*/
case 4:
value = "NaN";
if (energy_month_value >= 0.0 && energy_month_value < 1000.0)
{
if		(energy_month_value < 100.0)  value =  String(energy_month_value, 2);
else if (energy_month_value < 1000.0) value =  String(energy_month_value, 1);
}
Format ();
break;

/*Стоимость*/
case 5:
value = "NaN";
if (rate >= 0.0 && rate < 10000.0)
{
if		(rate < 1000.0)	  value =  String(rate, 1);
else if (rate <	 10000.0) value =  String(rate, 0);
}
Format ();
break;
}
}
//Начало обработки скроллинга
//Вывод единицы измерения
//Цвет текста из массива, цвет заливки экрана
tft.setTextColor(unit_text_color [parameter], color);
//Размер текста из массива
tft.setTextSize(unit_text_size [parameter]);
//Установка курсора, к Y прибавляется вычисляемое в Format () смещение
tft.setCursor(unit_x, counter_y + unit_y_offset);
//Вывод на экран строки с единицей измерения
tft.print (unit);

//Вывод линии подчеркивания
tft.drawFastHLine(10, counter_y + 120 - 2, 220, color_line);
//Вывод невидимого прямоугольника стирающего линию
tft.fillRect(5, counter_y + 120 - 2 + 6, 230, 3, color);

//Вывод значение параметра
//Цвет текста из массива, цвет заливки экрана
tft.setTextColor(value_text_color [parameter], color);
//Размер текста из массива
tft.setTextSize(value_text_size [parameter]);
//Установка курсора, к Y прибавляется вычисляемое в Format () смещение
tft.setCursor(value_x, counter_y + value_y_offset);
//Вывод на экран строки со значением параметра
tft.print (value);

//Остановка скроллинга на время таймера timerMotion, выполнение обмена во время остановки
//При счетчике координаты Y примерно в начале экрана и разрешенном запуске таймера
if (counter_y == 2 && flag_timerenable)
{
//Сбрасывается флаг разрешения изменения координаты Y, происходит остановка перемещения строк
flag_motion = false;
//Сброс флага отслеживающего однократность команды на запуск таймера
flag_timerenable  = false;
//Процедура получения и обработки данных с PZEM, передачи данных в Internet, переподключений при необходимости
Interchange();
//Запуск таймера определяющего время прекращения движения строк
xTimerChangePeriod(timerMotion, pdMS_TO_TICKS(int(round(delay_scroll*1000.0))) , 0);
//В параметре 0 дополнительно передача данных на ThingSpeakSend. Таймер в это время уже считает для уменьшения общего времени задержки
if (parameter == 0) ThingSpeakSend ();
}
//Уменьшение координаты Y на 7 пикселей при разрешенном движении. Максимально допустимое значение при использовании текста размером 7
if (flag_motion) counter_y = counter_y - 7;
//При уходе всех строк за верхнюю границу экран возврат указателя на нижнюю границу
//Разрешение обработки следующего параметра, разрешение запуска таймера
if (counter_y < -215) {counter_y = 240; flag_update = true; parameter++; flag_timerenable  = true;}
/*>-----------< PZEM >-----------<*/

/*Сброс сторожевого таймера*/
xTimerReset(timerWatchDog, 0);
}  



/*>-----------< Функции >-----------<*/
/*Получение и обработка данных от PZEM и Internet функции*/
 void Interchange ()
 {
  //Мгновенная мощность в W, при превышении уставки в kW отправка сообщения. Флаг обеспечивает не более одного сообщения в час
  power = pzem.power();
  if (power/1000.0 > alarm_power && flag_alarm_power) {flag_alarm_power = false; IFTTTSend (String(ifttt_event), String(board_name) + " " + String(F("High power! ")), String(" ") + String (power/1000.0, 1) + String(" kWh"), String(""));}
  //Электроэнергия в kWh
  energy				= pzem.energy();
  //Текущее значение электроэнергии, то что насчитал PZEM плюс коррекция
  energy_current_value = energy_correction_value + energy;
  //Текущее значение электроэнергии с начала месячного периода, общее текущее значение минус сохраненное на начало отсчета значение
  energy_month_value = energy_current_value - energy_before_value;
  //Вычисление стоимости, если расход не превышает социального лимита (130kWh)
  if (energy_month_value < energy_low_value) rate = price_low * (energy_month_value);
  //Вычисление стоимости, если расход превышает социальный лимит (130kWh)
  else rate = price_high * (energy_month_value - energy_low_value) + price_low * energy_low_value;
  //Стоимость в руб., при превышении уставки в отправка сообщения. Флаг обеспечивает не более одного сообщения в час
  if (rate > alarm_rate && flag_alarm_rate) {flag_alarm_rate = false; IFTTTSend (String(ifttt_event), String(board_name) + " " + String(F("High Rate! ")), String(" ") + String (rate, 1) + String(" RUR"), String(""));}
  //Напряжение
  voltage				= pzem.voltage();
  //Сила тока
  current				= pzem.current();
  //Коэффициент мощности
  pf					= pzem.pf();
  //Частота
  frequency = pzem.frequency();
  //Проверка и переподключение WiFi и MQTT
  WiFiReconnect ();
  //Отправка значений по MQTT
  mqtt_Send ();
  //Обновление значения времени по NTP
  GetTime();
  //Извлечение минут
  strftime(myStr, 3, "%M", &timeinfo);
  minute  = String(myStr).toInt();
  //Извлечение часов
  strftime(myStr, 3, "%H", &timeinfo);
  hour    = String(myStr).toInt();
  //Извлечение дня даты
  strftime(myStr, 3, "%d", &timeinfo);
  day     = String(myStr).toInt();
  //Извлечение номера месяца даты
  strftime(myStr, 3, "%m", &timeinfo);
  month   = String(myStr).toInt();
  //Повторяется в начале каждого часа, если текущее значение минут меньше предшествующего
  if (minute < saved_minute)
  {
      //Запоминание минимально возможного значения для исключения повторного входа в это условие
      saved_minute = 0;
      //Разрешение повторной отправки ежечасовогого уведомления
      flag_alarm_power = true;
      //Проверка переполнения счетчика киловат-часов PZEM
      //При приближении к переполнению отправка уведомления, сохранение нового значения коррекции, сброс счетчика PZEM в 0
      if (energy >= 9900.0) {IFTTTSend (String(ifttt_event), String(board_name) + " " + String(F("Counter overflow")), String(""), String(energy)); energy_correction_value = energy_correction_value + energy; memory.putFloat("correction", energy_correction_value); pzem.resetEnergy();}
  }
  //Текущее значение минут не меньше предшествующего, сохранение текущего значения минут
  else saved_minute = minute;

  //Повторяется один раз в месяц в день равный дню предшествующего снятия показаний, если текущее значение дня увеличилось (начало суток)
  if (day == day_before && day > saved_day)
  {
      //Запоминание максимально возможного значения для исключения повторного входа в это условие
      saved_day = 31;
      //Разрешение повторной отправки ежесуточного уведомления
      flag_alarm_rate = true;
      //Установка показаний  начала отсчета месячного периода
      energy_before_value = energy_current_value;
      memory.putFloat("before", energy_before_value);
      memory.putInt("day", day_before);
      //Запоминание номера текущего месяца
      month_before = month;
      memory.putInt("month", month_before);
      //Отправка уведомления
      IFTTTSend (String(ifttt_event), String(board_name) + " " + String(F("Month, starting point")), String(" ") + String (energy_before_value, 1), String(""));
  }
  //Текущий день не равен назначенному или он не увеличился с прошлой проверки, сохранение значения дня текущей даты
  else saved_day = day;
}
/**/




/*Форматирование  и позиционирование строк выводимых на экран*/
void Format ()
{
  //Загрузка из массива наименования единицы измерения
  unit = unit_name [parameter];
  //Загрузка из массива размера шрифта единицы измерения
  tft.setTextSize(unit_text_size [parameter]);
  //Получение размеров строки
  tft.getTextBounds(unit, 0, 0, &x_null, &y_null, &width_1, &height_1);
  //Вычисление смещения строки по Y
  unit_y_offset = (240/3-height_1)/2 + 18;
  //Вычисление смещения строки по X, центрирование на экране
  unit_x = (240-width_1)/2;
  //Загрузка из массива цвета разделительной линии
  color_line = unit_text_color [parameter];
  //Загрузка из массива размера шрифта величины параметра
  tft.setTextSize(value_text_size [parameter]);
  //Получение размеров строки
  tft.getTextBounds(value, 0, 0, &x_null, &y_null, &width_2, &height_2);
  //Вычисление смещения строки по Y
  value_y_offset = 2*240/3 - 5;
  //Вычисление смещения строки по X, центрирование на экране
  value_x = (240-width_2)/2;
}
/**/

/*Функция для таймера задержки*/
//Разрешение изменения координаты Y (скроллинга) после паузы, определенной таймером
void Delay_motion () {flag_motion = true;}
/**/
/*>-----------< Функции >-----------<*/
