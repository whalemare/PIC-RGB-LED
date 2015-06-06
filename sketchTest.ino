  #define rangePin A0 // датчик препятствия Sharp
  #define bottomRange 620 // нижний предел 
  // верхний "нулевой" порог, (при отсутсвии препятсвия), высчитывается через функцию topDistance()
  #define delta 20 // стандартно возможное отклонение
  #define timeOutRange 100 // 1 считывание раз в timeOutRange мс
  #define timeOutHand 1500 // задержка чтобы убрать руку мс

  #define redPin 9 // красный подключен к 9 пину
  #define greenPin 10 // зеленый подключен к 10 пину
  #define bluePin 11 // синий подключен к 11 пину

  unsigned long timerRange, prevRange, nowRange; // переменные для вычисления расстояния
  int redNum, greenNum, blueNum; // [0;255] значения цветов RGB
  int topRange; // верхний порог
  int i=0; // для счетчика
  int timeSec; // время руки над датчиком

  // lightMode переделана в функцию. Режим лампы в настоящий момент (0-ночник, 1 светильник, 2 радуга, 8 отключение)
  byte valueRGB = 0; // яркость или оттенок (нормализованное значение nowRange)
  byte timeout = 60; // итераций чтобы выключить светильник
  byte prevValueRGB = 0; // прошлое значение яркости 
  byte factor = 0; // для изменения яркости
  byte lmNum = 1;

  int testCount;

  void setup() { 
    Serial.begin(9600);
    pinMode(rangePin, INPUT); // шарп на вход
    pinMode(redPin, OUTPUT); // redPin на выход
    pinMode(greenPin, OUTPUT); // greenPin на выход
    pinMode(bluePin, OUTPUT); // bluePin на выход

    timerRange = millis(); // старт таймера на периодическое считывание дистанции

    nowRange = getRange(); // первое вычисление дистанции
    topRange = topDistance(); // вычисление верхнего порога
    randomSeed(analogRead(0)); // для рандомайзера
  }

  void loop()
  {
  // ВЫЧИСЛЕНИЕ ДИСТАНЦИИ РАЗ В timeOutRange раз (+)

    if ((millis() - timerRange) > 10) // 1. вычисление дистанции раз в timeOutRange
    { 
      prevRange = nowRange; // память предыдущего значения дистанции
      nowRange = getRange(); // текущее значение дистанции
      timerRange = millis(); // сброс таймера интервала вычисления дистанции

      println("Range: ", nowRange);
    }

  // Для включения радуги
   if (lmNum==1 || lmNum==2) // если работает светильник
   {
     if (nowRange<=topRange-10) // если рука выше позиции, получаемой если нет препятствий
       {
           lightMode(2);
            delay(100);
       }

   }


  // Если дистанция в допустимых пределах
    if ((nowRange < bottomRange - delta) && (nowRange > topRange+delta))
    {
      // Для valueRGB: 58-66 строки
      if (nowRange > bottomRange - delta)
        valueRGB = 0;
      else{
        if (prevValueRGB==0)
          prevValueRGB = 255;
        else
          prevValueRGB = valueRGB;
        valueRGB = 254 - map(nowRange, topRange, bottomRange, 0, 254); // приведение текущего значения к диапазону 0 - 254 и "переворот" 
      }

    /*
            Serial.print("prevRGB: ");
            Serial.println(prevValueRGB);

            Serial.print("valueRGB: ");
            Serial.println(valueRGB);
    */
      timeSec = counter(); // сколько рука была над датчиком

      if (lmNum==0) // если лампа выключена
        if (timeSec>=3) // проведи рукой, чтобы ее включить
          firstOn();
    }

  } // loop{...}

  // ФУНКЦИИ ---------------------------------------------------------------------

  // #1. Вычисление дистанции
    unsigned int getRange()
    {
      byte i;
      unsigned int rangeFinder = 0;

      for (i = 0; i < 100; i++)
        rangeFinder = rangeFinder + analogRead(rangePin);
      
      rangeFinder = rangeFinder/100;
      return rangeFinder;
    }

  // #2. Установка цвета
    void setColor(int redNum, int greenNum, int blueNum)
    {
      analogWrite(redPin, redNum);
      analogWrite(greenPin, greenNum);
      analogWrite(bluePin, blueNum);
        /*
        Serial.println(redNum);
        Serial.println(greenNum);
        Serial.println(blueNum);
        Serial.println();
        */  
    }

  // #3. Настройка верхнего порога c цветовой подсказкой об окончании
    unsigned int topDistance()
    {
      int i, topR, redColor=255;
      int *all = new int[topR];

      Serial.println("PROCESSING...");
      for (i=0; i<=20; i++) // 2100 мс
      {
      //  setColor(redColor, 207, 232); // устанавливаем цветовую подсказку
        all[i]=getRange(); // массив с дистанциями
        topR += all[i];
      //  redColor = abs(redColor-15);
      //    Serial.print("redColor: "); Serial.println(redColor);
        delay(100);   
      } 

      topR = topR/i; // среднее арифметическое
      if (topR>250) // если верхний предел слишком мал
      {
        for (i=0; i<=255; i++)
          setColor(i, i, i);
      }
      else // если все хорошо :)
        firstOn(); // #7

      Serial.print("topRange: "); Serial.println(topR);
      return topR;
    }

  // #4. lightMode - Виды работ лампы
    void lightMode(int number)
    {
      int fade; // для затухания в режиме 0
      switch(number) 
      {
        case 1: // включен светильник
          lmNum = 1;
          break;
        case 2: // помигать светодиодиком
        lmNum = 2;
          testLED();
          break;
        case 0: // выключение светильника
          lmNum = 0; // для индикации в других частях программы
          /*
          TODO если включен не белый цвет, то перед выключением буратинка перекинет цвета в белый
          и только потом начнется затухание. Сделать, чтобы затухание работало для всех цветов
          */
          for (fade=255; fade>0; fade--)
          {
            setColor(fade, fade, fade);
            delay(20);
            if (fade==1) // TODO костыль. fade всегда доходит только до 1, не нижее
              setColor(0, 0, 0);
          }
          delay(2000);
          break;
        default: // если что то пошло не так
          setColor(255, 0, 0);
          delay(200);
          setColor(0, 255, 0);
          delay(159);
          setColor(0, 0, 255);
          delay(200);
          setColor(0, 0, 0);
          break;
      }
    }

  // #5temp. Тест LED
    void testLED()
    {
      byte a, b, c;
        a=random(256);
        b=random(256);
        c=random(256);
        setColor (a, b, c);
      delay(500);
    }

  // #6. Нечто напоминающее таймер
    //     3.7 cекунд = 100 итераций -> 1 секунда = 27 итераций
    int counter()
    {
       int iCount=0;
       while ((nowRange <= getRange()+40) && (nowRange>=getRange()-40)) // если руку подвели
       {
          if ((getRange() <= topRange+delta) && (getRange() >= topRange - delta)) // если рука убрана
              return iCount;

          iCount++; // +1

          if (iCount >= timeout) // если время дошло до выключения светильника
            {
              lightMode(0);
              iCount=0;
              break;
            }
          Serial.print("iCount="); Serial.println(iCount);    
       }
       return iCount;
    }

  // #7. Первое включение
    void firstOn()
    {
      int force;
      for (force=0; force<=253; force+=2) // потому что переполнение и лента "вырубается"
      {
        if (force>80)
          force++;
        if (force>150)
          force++;
        setColor(force, force, force);
        //Serial.print("force"); Serial.println(force);
        delay(30); // TODO переделать в прерывания
      }
      lightMode(1); // мод светильника #4.
    }

  // #8. Для удобной печати
    void println(char text[60], int number)
    {
      Serial.print(text);
      if (number!=0)
          Serial.println(number);
    }

