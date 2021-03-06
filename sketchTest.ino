#define rangePin A0 // датчик препятствия Sharp
#define bottomRange 620 // нижний предел 
#define maxRange 120 // верхний предел
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
int upTempTange=0; // для upTempTange = nowRange, для правильной работы в промежутке (topRange; maxRange)

// lightMode переделана в функцию. Режим лампы в настоящий момент (1 светильник, 2 радуга, 3 ночник 8 отключение)
byte valueRGB = 0; // яркость или оттенок (нормализованное значение nowRange)
byte timeout = 60; // итераций чтобы выключить светильник
byte prevValueRGB = 0; // прошлое значение яркости 
byte factor = 0; // для изменения яркости
byte lmNum = 1; // лампа в режиме светильниа
byte r = 255; // для смены яркости в диапазоне радуги
byte rdd; // для проверки значения красного цвета

int testCount;

void setup() 
{ 
  Serial.begin(9600); // для отладки
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

// Если лампа полностью накрыта ладонью
	if (((nowRange >= 350) && (nowRange<=700)) || ((nowRange>=1) && (nowRange<=10)))
	{	
		println("No4nik?", 0);
		delay(400);

		if (((getRange() >= 250) && (getRange()<=700)) || ((getRange()>=1) && (getRange()<=10)))
		{
			println("Yes", 0);
			lightMode(3); // включаем ночник
		}
		else
		println("No", 0);
	}

// Если дистанция в допустимых пределах

    if ((nowRange >= topRange+20) && (nowRange <= 350))
    {
        upTempTange = nowRange;
        println("We are in cycl >> ", nowRange);
        delay(1000); 
          if ((getRange() <= upTempTange+10) && (getRange() >= upTempTange-10)) // если рука не сдвинулась за секунду
          {
          	if (lmNum==1) // если был светильник
          	{          
          		println("RADUGA", getRange());
            	lightMode(2);
          	}

          	if ((lmNum==3) || (lmNum==0)) // если был ночник или лампа выключена
          	{
          		println("LIGHT", 0);
          		lightMode(1);
          		delay(2000);
          	}

          }
          else // иначе, если рука сдвинулась
          {
            println("TROLOLO", getRange());
          }
    }

/* dfgsdfgsdfg
	if ((nowRange < 400) && (nowRange > 180))
	{
	  timeSec = counter(); // сколько рука была над датчиком

	  if (lmNum==0) // если лампа выключена
	    if (timeSec>=1) // проведи рукой, чтобы ее включить
	      firstOn();
	}
*/

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
  	rdd=redNum;
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
    if (topR>=150) // если верхний предел слишком мал
    {
    	for (int i=0; i<3; i++) // помигаем красным цветом, говоря, что нужно переставить лампу
    	{
	      setColor(192,17,29);
	      delay(300);
	      setColor(0,0,0);
	      delay(100);
  		}
    }
    else // если все хорошо :)
      firstOn(); // #7

    println("topRange", topR);
    return topR;
  }

// #4. lightMode - Виды работ лампы
  void lightMode(int number)
  {
    int fade=255, fadeEnd=100; // для затухания в режиме 0
    int kaka=0;
    switch(number) 
    {
      case 1: // включен светильник
        lmNum = 1;
        if (rdd!=255) // если вдруг свет тусклый
		for (int i=rdd; i<255; i++)
		{
			setColor(i, i, i);
			delay(30);
		}
        break;
      case 2: // радуга
      lmNum = 2;
        rainbowing();
        break;
      case 3: // ночник
        lmNum = 3;
      	while (((getRange() >= 200) && (getRange()<=700)) || ((getRange()>=1) && (getRange()<=10)))
      	{
      		if (rdd>=2)
      		{
      			println("RDD--", rdd);
				rdd--;
				setColor(rdd, rdd, rdd);
				delay(20);
      		}
      		else
      		{
      			setColor(0,0,0);
      			lmNum=0;
      		}
      	}
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

        if ((iCount >= timeout) && (lmNum!=0)) // если время дошло до выключения светильника
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
    else
        Serial.println();
  }

/* почти работает. Осталось прикрутить смену яркости ко всем цветам
    и сделать запоминание выбранной яркости
  // #9. Cмена яркости
    void brightness()
    {
      boolean exiting = false;
      do {
       if ((millis() - timerRange) > 10) // 1. вычисление дистанции раз в timeOutRange
          { 
           prevRange = nowRange; // память предыдущего значения дистанции
           nowRange = getRange(); // текущее значение дистанции
           timerRange = millis(); // сброс таймера интервала вычисления дистанции

           println("Rangeing: ", nowRange);
          }

        if (nowRange==1 || nowRange==2 || nowRange == 3 || nowRange == 4)
          exiting = true;

        if (prevValueRGB==0)
              prevValueRGB = 255;
            else
              prevValueRGB = valueRGB;
        valueRGB = 254 - map(nowRange, 130, bottomRange-60, 0, 254); // приведение текущего значения к диапазону 0 - 254 и "переворот" 

        Serial.print("9rkost: "); Serial.println(valueRGB);
        setColor(valueRGB, 0, 0);  

        } while (exiting !=true);
    }
*/

// #10. Радуга
  int rainbowing()
  {
    lmNum = 2; // чтобы не конфликтовало с выключением лампы
	int center;
    byte rgbColour[3], fade;
    boolean exx = false;

	if (rdd!=255) // если вдруг свет тусклый
		for (int i=rdd; i<255; i++)
		{
			setColor(i, i, i);
			delay(30);
		}

    for (fade=rdd; fade>0; fade--)  // плавно перейдем к красному цвету
    {
      setColor(255, fade, fade);
      delay(20);
      if (rdd!=255)
      	delay(50);
      if (fade==1) // TODO костыль. fade всегда доходит только до 1, не нижее
        setColor(0, 0, 0);
    } 

    while (exx==false) // пока рак на горе не свиснет
    {
        // Начинаем с красного
        rgbColour[0] = 255;
        rgbColour[1] = 0;
        rgbColour[2] = 0;  
   
       
        // Choose the colours to increment and decrement.
        for (int decColour = 0; decColour < 3; decColour += 1) 
        {
          int incColour = decColour == 2 ? 0 : decColour + 1;
          
          // cross-fade the two colours.
          for(int i = 0; i < 255; i += 1) 
          {
            rgbColour[decColour] -= 1;
            rgbColour[incColour] += 1;
            setColor(rgbColour[0], rgbColour[1], rgbColour[2]);

            if (getRange() >= topRange+30)
            {
              	// возвращаемся к белому цвету
              	println("Go to the white", 0);
              	while (exx == false)
              	{
	           	    println("0:", rgbColour[0]);
              		println("1:", rgbColour[1]);
              		println("2:", rgbColour[2]);
								// TODO переделать в цикл
              		if (rgbColour[0]!=255)
              			rgbColour[0]++;

              		if (rgbColour[1]!=255)
              			rgbColour[1]++;

              		if (rgbColour[2]!=255)
              			rgbColour[2]++;
              					//
              		setColor(rgbColour[0], rgbColour[1], rgbColour[2]);
              		delay(40);

	                if ((rgbColour[0] == 255) && (rgbColour[1] == 255) && (rgbColour[2] == 255))
	          		{
	          			lightMode(1); // индикатор Включенной лампы
					    exx == true;
				        return 0;
	            	}

          		}

            }
            delay(50);
          }
        }
    }          
  }
/*
	+Добавлена цветовая подсказка, если над лампой не потолок или рассчет расстояния был неверен
	+В очередной раз изменены диапазоны
	+Добавлен ночник
	+Добавлен переход в Радугу из ночника, при этом свет плавно выкручивается до максимума, а потом переходит в радугу
	+Значительно пересмотрена структура кода и поведение лампы, к примеру
		теперь нет ночника, и чтобы выставить яркость нужно задержать руку внизу лампы
		светильник и радуга меняются циклически, поочередно
*/