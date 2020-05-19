//  Controlo de abertura da estufa Manual ou Automático(por temperatura)
//  Pedro Garrido Set-Out 2018
//  V2.0 mar19

const int ABRIR = 3;   // Pino 3   - ABRIR  (ativo a baixo)
const int FECHAR = 11; // Pino 11  - FECHAR (ativo a baixo)

#include <dht.h>
dht DHT;
#define DHT_PIN 2 // Pino 2  - Sensor Temperatura/Humidade

int estadoJanela = 1; // 0-Janela Aberta, 1-Janela Fechada

int temp, temp1, humid, humid1, tempMin, tempMax, humidMin, humidMax;
int modo = 1;    // 0-MANUAL 1-AUTO
int escolha = 1; // 0-escolha Min  1-escolha Max
int minimo = 17; // temp Mínima
int maximo = 22; // temp Máxima
int tempoabertura = 10000;
int tempofecho = 10000;

int visualiza = 0; // 0- visualiza TEMP   1- visualiza HUMID

unsigned long previousMillis = 0;
const long interval = 5000; // intervalo de vizualização
unsigned long previousMillisBack = 0;
const long intervalBack = 60000; // intervalo de vizualização backlight
int Back = 100;                  // intensidade backlight
int Back2 = 3;                   // intensidade backlight nivel baixo

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
// define some values used by the panel and buttons
int lcd_key = 0;
int adc_key_in = 0;
#define btnRIGHT 0
#define btnUP 1
#define btnDOWN 2
#define btnLEFT 3
#define btnSELECT 4
#define btnNONE 5

int read_LCD_buttons()
{
    adc_key_in = analogRead(0); // read the value from the sensor
    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    if (adc_key_in > 1000)
        return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
    // For V1.1 us this threshold
    if (adc_key_in < 50)
        return btnRIGHT;
    if (adc_key_in < 250)
        return btnUP;
    if (adc_key_in < 450)
        return btnDOWN;
    if (adc_key_in < 650)
        return btnLEFT;
    if (adc_key_in < 850)
        return btnSELECT;

    return btnNONE; // when all others fail, return this...
}

void setup()
{
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print(" CTRL DE ESTUFA");
    lcd.setCursor(0, 1);
    lcd.print("Pedro Garrido 18");
    delay(3000);

    pinMode(ABRIR, OUTPUT);
    pinMode(FECHAR, OUTPUT);
    digitalWrite(ABRIR, HIGH);
    digitalWrite(FECHAR, HIGH);

    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    // lcd.print("MANUAL           ");  //arranque em modo MANUAL
    lcd.print("AUTO Min" + String(minimo) + "~Max" + String(maximo)); //arranque em modo AUTO

    pinMode(10, OUTPUT); // Backlight
    analogWrite(10, Back);

    int chk = DHT.read22(DHT_PIN);
    temp = DHT.temperature;
    humid = DHT.humidity;

    tempMin = temp;
    tempMax = temp;
    humidMin = humid;
    humidMax = humid;
    lcd.setCursor(0, 1);
    lcd.print("TEMP  " + String(temp) + (char)223 + "C " + String(tempMin) + "-" + String(tempMax) + "       ");
}

void loop()
{
    delay(50);

    if (modo == 1)
    { //modo automático
        if (temp > maximo and estadoJanela == 1)
        { //chegou ao max e janela fechada
            lcd.setCursor(0, 1);
            lcd.print("ABRIR JANELA ...");
            digitalWrite(ABRIR, LOW);
            delay(tempoabertura);
            estadoJanela = 0; //janela aberta
            digitalWrite(ABRIR, HIGH);
            lcd.setCursor(0, 1);
            lcd.print("JANELA ABERTA   ");
            delay(1000);
        }
        if (temp < minimo and estadoJanela == 0)
        { //chegou ao min e janela aberta
            lcd.setCursor(0, 1);
            lcd.print("FECHAR JANELA...");
            digitalWrite(FECHAR, LOW);
            delay(tempofecho);
            estadoJanela = 1; //janela fechada
            digitalWrite(FECHAR, HIGH);
            lcd.setCursor(0, 1);
            lcd.print("JANELA FECHADA  ");
            delay(1000);
        }
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillisBack >= intervalBack)
    { // Desligar Backlight
        previousMillisBack = currentMillis;
        analogWrite(10, Back2);
    }
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;

        int chk = DHT.read22(DHT_PIN); // LER Temperatura e Humidade

        temp1 = DHT.temperature;
        humid1 = DHT.humidity;

        if (temp1 >= 0 and temp1 < 100)
            temp = temp1; //temperaturas dentro do limite?
        if (humid1 >= 0 and humid1 < 100)
            humid = humid1; //humidades dentro do limite?

        if (tempMin > temp)
            tempMin = temp;
        if (tempMax < temp)
            tempMax = temp;
        if (humidMin > humid)
            humidMin = humid;
        if (humidMax < humid)
            humidMax = humid;

        lcd.setCursor(0, 1);

        if (visualiza == 0)
        {
            visualiza = 1;
            lcd.print("TEMP  " + String(temp) + (char)223 + "C " + String(tempMin) + "-" + String(tempMax) + "       ");
        }
        else
        {
            visualiza = 0;
            lcd.print("HUMID " + String(humid) + "%  " + String(humidMin) + "-" + String(humidMax) + "       ");
        }
    }

    lcd_key = read_LCD_buttons();
    digitalWrite(ABRIR, HIGH);
    digitalWrite(FECHAR, HIGH);
    switch (lcd_key)
    {
    case btnRIGHT: //  Clear tempMin,tempMax,humidMin,humidMax
    {
        analogWrite(10, Back); // Ligar Backlight
        delay(200);
        break;
    }
    case btnLEFT: // Escolher Min / Max
    {
        analogWrite(10, Back); // Ligar Backlight
        previousMillisBack = currentMillis;

        if (modo == 1 and escolha == 0)
        {
            escolha = 1;
            lcd.setCursor(4, 0);
            lcd.print(" ");
            lcd.setCursor(10, 0);
            lcd.print("~");
            delay(200);
            break;
        }

        if (modo == 1 and escolha == 1)
        {
            escolha = 0;
            lcd.setCursor(10, 0);
            lcd.print(" ");
            lcd.setCursor(4, 0);
            lcd.print("~");
            delay(200);
        }
        break;
    }
    case btnUP: //  Aumentar / Abrir
    {
        if (modo == 0)
        { // Abrir
            digitalWrite(ABRIR, LOW);
            delay(100);
            break;
        }

        analogWrite(10, Back); // Ligar Backlight
        previousMillisBack = currentMillis;

        if (modo == 1 and escolha == 0)
        {
            if ((maximo - minimo) > 1)
                minimo = minimo + 1;
            lcd.setCursor(8, 0);
            lcd.print(minimo);
        }

        if (modo == 1 and escolha == 1)
        {
            maximo = maximo + 1;
            lcd.setCursor(14, 0);
            lcd.print(maximo);
        }
        delay(200);
        break;
    }
    case btnDOWN: //  Diminuir / Fechar
    {
        if (modo == 0)
        { // Fechar
            digitalWrite(FECHAR, LOW);
            delay(100);
            break;
        }

        analogWrite(10, Back); // Ligar Backlight
        previousMillisBack = currentMillis;

        if (modo == 1 and escolha == 0)
        {
            minimo = minimo - 1;
            lcd.setCursor(8, 0);
            lcd.print(minimo);
            if (minimo < 10)
                lcd.print(" ");
        }

        if (modo == 1 and escolha == 1)
        {
            if ((maximo - minimo) > 1)
                maximo = maximo - 1;
            lcd.setCursor(14, 0);
            lcd.print(maximo);
            if (maximo < 10)
                lcd.print(" ");
        }
        delay(200);
        break;
    }
    case btnSELECT: //  Modo MANUAL/AUTO
    {
        analogWrite(10, Back); // Ligar Backlight
        previousMillisBack = currentMillis;

        if (modo == 1)
        {
            modo = 0;
            lcd.setCursor(0, 0);
            lcd.print("MANUAL           ");
        }
        else
        {
            modo = 1;
            lcd.setCursor(0, 0);
            lcd.print("AUTO Min" + String(minimo) + " Max" + String(maximo));
            if (escolha == 1)
            {
                lcd.setCursor(10, 0);
                lcd.print("~");
            }
            else
            {
                lcd.setCursor(4, 0);
                lcd.print("~");
            }
        }
        delay(200);
        break;
    }
    case btnNONE:
    {
        break;
    }
    }
}
