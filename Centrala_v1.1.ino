////////////////////////////////////////////////BIBLIOTEKI///////////////////////////
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
///////////////////////////////////////////////DEKLARACJA PINOW/////////////////////////

#define PIR1  6      //pin trig PIR
#define PIR2  15      //pin trig PIR


#define gsmTx 2
#define gsmRx 3
#define sygnal 14
#define backlight 17


//////////////////////////////////////////////GSM//////////////////////////////////
char odczyt[200];
SoftwareSerial gsm(gsmTx, gsmRx);
String telefon = "+";
String telefon2 = "+";
int flaga_sms = 0;

/////////////////////////////////////CZUJKI///////////////////////////////////////

boolean flaga_czujnika;

//////////////////////////////////KLAWIATURA/////////////////////////////////////

char keys[4][3] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte pinyRzedu[4]   = {7, 8, 9, 10};
byte pinyKolumny[3] = {11, 12, 13};
Keypad keypad = Keypad(makeKeymap(keys), pinyRzedu, pinyKolumny, 4, 3);
char key;

String KOD = "1234#";
String PIN = "##";

String kod;
String pin;
String nowy_kod;

//////////////////////////////////////LCD/////////////////////////////////////

LiquidCrystal_I2C lcd(0x3F, 2,  1,  0,  4,  5,  6,  7,  3, POSITIVE); 

String nowy_kod_ekran;
String pin_ekran;
String kod_ekran;

////////////////////////////////////ZMIENNE POMOCNICZE//////////////////////////////

unsigned long timer_ukrycia_kodu, timer_przelaczenia, timer_trybu, timer_ukrycia_pinu, timer_ukrycia_nowego_kodu, timer_wyjscia, timer_rozbrojenia, timer_alarmu, timer_opuszczenia, timer_opuszczenia2,  minuty, sekundy;
boolean kod_prawidlowy = false, alarm_uzbrojony = true, flaga_alarmu = false, flagax = true;
int i = 1, tryb = 0, j = 1;
int x = 0, czas,  flaga_uzbrojenia;
boolean alarm_wyzwolony_przod, alarm_wyzwolony_tyl;


////////////////////////////////////////////////////////////////////////////////

void setup() {
  delay(5000); //czas na zalogowanie gsm
  pinyPIR();
  lcd.begin(16, 2);
  digitalWrite(backlight,HIGH);
  Serial.begin(9600);
  pinMode(sygnal, OUTPUT);
  pinMode(backlight, OUTPUT);
  gsm.begin(9600);
  delay(200);
  gsmCommand("AT+CMGF=1");
  delay(100);
  gsmCommand("AT+CSCS=\"GSM\"");
  delay(100);
  gsmSMS(telefon, "Inicjalizacja alarmu");
  delay(5000);
  gsmSMS(telefon2, "Inicjalizacja alarmu");
  


}

/////////////////////////////////PROGRAM GLOWNY//////////////////////////////////


void loop() {

  while(alarm_uzbrojony){

  
  odczytanie();
//Serial.println(odczyt);
  if (strstr(odczyt, "+CMTI: \"SM\",1") && flaga_sms == 0) {
    gsm.println("AT+CMGR=1");
   // delay(10);
    flaga_sms = 1;
    
  } 
  else if (strstr(odczyt, "+CMTI: \"SM\"") && flaga_sms == 0){
     gsm.println("AT+CMGDA=\"DEL ALL\"");
  }


  if ((strstr(odczyt, "600013973") || strstr(odczyt, "504117487")) && strstr(odczyt, "Off") && flaga_sms == 1) {

    kod = KOD;

    delay(100);
    flaga_sms = 2;
  }


  if (  ((!strstr(odczyt, "600013973") || !strstr(odczyt, "504117487"))  || ((strstr(odczyt, "600013973") || !strstr(odczyt, "504117487")) && !strstr(odczyt, "Off"))) && (!strstr(odczyt, "AT+CMGR=1")) && (!strstr(odczyt, "+CMTI: \"SM\",1"))  && flaga_sms == 1) {
    gsm.println("AT+CMGDA=\"DEL ALL\"");
    flaga_sms = 0;
   // delay(10);
  }

  if (flaga_sms == 2) {
    gsm.println("AT+CMGDA=\"DEL ALL\"");
    gsm.flush();
    flaga_sms = 0;
    delay(100);
    gsmSMS(telefon, "Alarm rozbrojony zdalnie");
    delay(5000);
    gsmSMS(telefon2, "Alarm rozbrojony zdalnie");
  }
    
  //////////////////////////////////11GDY ALARM UZBROJONY11//////////////////////////
  if (!kod_prawidlowy && alarm_uzbrojony) {
    if (flaga_czujnika == 1) {
      alarm_wyzwolony_przod = czujniki_odczyt(PIR2);
//Serial.print("PRZOD:");
//Serial.println(digitalRead(PIR2));
    }
    if (flaga_czujnika == 0) {
      
    alarm_wyzwolony_tyl = czujniki_odczyt(PIR1);
//Serial.print("TYL:");
//Serial.println(digitalRead(PIR1));
    }
    flaga_czujnika = !flaga_czujnika;
    if (alarm_wyzwolony_przod || alarm_wyzwolony_tyl) {
      while (i == 1) {
        timer_rozbrojenia = millis();
        i = 0;
        digitalWrite(backlight,HIGH);
        flaga_alarmu = 1;

      }
    }

    timer_alarmu = (60 - ((millis() - timer_rozbrojenia) / 1000));
    if (timer_alarmu > 500) timer_alarmu = 0;
    minuty = floor(timer_alarmu / 60);

    sekundy = timer_alarmu - (minuty * 60);

    if (timer_alarmu == 0 && flaga_alarmu) {
      digitalWrite(A0, HIGH);
      while (j == 1) {
        gsmSMS (telefon, "Wywolano alarm!!");
        delay(5000);
        gsmSMS (telefon2, "Wywolano alarm!!");
        j = 0;
      }
    }



    key = keypad.getKey();
    if (key) {
      kod = String(kod + key);
      kod_ekran = String(kod_ekran + key);
      timer_ukrycia_kodu = millis();
      if (key == '*') {
        kod = String("");
        kod_ekran = kod;
        lcd.clear();
      }
    }
    lcd.noCursor();
    lcd.setCursor(0, 0);
    lcd.print("Wpisz kod:");
    lcd.setCursor(12, 0);
    if (timer_alarmu >= 60) {
      lcd.print(minuty);
    }
    else lcd.print("0");
    lcd.setCursor(13, 0);
    lcd.print(":");
    if (sekundy >= 10) {
      lcd.setCursor(14, 0);
    }
    else {
      lcd.setCursor(14, 0);
      lcd.print("0");
      lcd.setCursor(15, 0);
    }
    lcd.print(sekundy);


    lcd.setCursor(0, 1);
    lcd.print(kod_ekran);
    lcd.cursor();

    if (millis() - timer_ukrycia_kodu > 400) {
      kod_ekran.replace('0', '*');
      kod_ekran.replace('1', '*');
      kod_ekran.replace('2', '*');
      kod_ekran.replace('3', '*');
      kod_ekran.replace('4', '*');
      kod_ekran.replace('5', '*');
      kod_ekran.replace('6', '*');
      kod_ekran.replace('7', '*');
      kod_ekran.replace('8', '*');
      kod_ekran.replace('9', '*');
      lcd.noCursor();

      lcd.setCursor(0, 1);
      lcd.print(kod_ekran);
      lcd.cursor();
    }









    if (kod == KOD) {
      kod_prawidlowy = true;
      timer_przelaczenia = millis();
      lcd.clear();
      digitalWrite(backlight,HIGH);
    }
    delay(10);
  }
  ////////////////////////////////////22ROZBROJENIE ALARMU22//////////////////
  if (kod_prawidlowy) {
    while (alarm_uzbrojony) {
      if (millis() - timer_przelaczenia < 2000) {
        lcd.noCursor();
        lcd.setCursor(0, 0);
        lcd.print("Kod prawidlowy!");
      }
      if (millis() - timer_przelaczenia > 2500) {

        lcd.setCursor(0, 0);
        lcd.print("Rozbrajam alarm");
        flaga_alarmu = false;
        alarm_uzbrojony = false;
        kod = String("");
        kod_ekran = String("");
        i = 1;
        digitalWrite(A0, LOW);
        j = 1;
        delay(2000);
      }
    }
  }
}
  //////////////////////////////////////////33GDY ALARM ROZBROJONY33///////////////////////////////////////
  while (!alarm_uzbrojony) {
    
  odczytanie();
Serial.println(odczyt);
  if (strstr(odczyt, "+CMTI: \"SM\",1") && flaga_sms == 0) {
    gsm.println("AT+CMGR=1");
   Serial.println("debug1");
    flaga_sms = 1;
 
  } 
  else if (strstr(odczyt, "+CMTI: \"SM\"") && flaga_sms == 0){
     gsm.println("AT+CMGDA=\"DEL ALL\"");
     
//   Serial.println("debug2");
  }


  if ((strstr(odczyt, "600013973") || strstr(odczyt, "504117487")) && strstr(odczyt, "On") && flaga_sms == 1) {
      tryb = 2;
      kod = KOD;
      digitalWrite(backlight,HIGH);
      delay(100);
      flaga_sms = 2;
      
//   Serial.println("debug3");
  }

delay(5);
  if (  ((!strstr(odczyt, "600013973") || !strstr(odczyt, "504117487"))  || ((strstr(odczyt, "600013973`") || !strstr(odczyt, "504117487")) && !strstr(odczyt, "On"))) && (!strstr(odczyt, "AT+CMGR=1")) && (!strstr(odczyt, "+CMTI: \"SM\",1"))  && flaga_sms == 1) {
     gsm.println("AT+CMGDA=\"DEL ALL\"");
    flaga_sms = 0;
  
    
//   Serial.println("debug4");
  }

  if (flaga_sms == 2) {
    gsm.println("AT+CMGDA=\"DEL ALL\"");
    gsm.flush();
    flaga_sms = 0;
    delay(100);
    gsmSMS(telefon, "Alarm uzbrojony zdalnie");
    delay(5000);
    gsmSMS(telefon2, "Alarm uzbrojony zdalnie");
 
//   Serial.println("debug5");
  }
 
  


    /**************************************44TRYB CZUWANIA44******************************************/
    if (tryb == 0) {


      lcd.clear();
      key = keypad.getKey();
      if (key == '*') {
        while (i == 1) {
          timer_trybu = millis();
          i = 0;
          digitalWrite(backlight,HIGH);
        }

        if (millis() - timer_trybu > 4000 && millis() - timer_trybu < 6000) {
          tryb = 1;

          x = 0;
          i = 1;
        }

      }
      if (key == '#') {
        if (millis() - timer_trybu > 2000 && millis() - timer_trybu < 10000) {
          tryb = 2;

          x = 0;
          i = 1;
        }

      }
      if (millis() - timer_trybu > 10000) {
        i = 1;
        digitalWrite(backlight,LOW);
      }
    }



    /**************************************55TRYB ZMIANY HASLA55******************************************/
    if (tryb == 1) {


      if (x == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Zmiana hasla");
        delay(2000);
        x = 1;
        lcd.clear();
      }
      /**************************************66WPISYWANIE PINU66******************************************/
      if (x == 1) {
        key = keypad.getKey();
        if (key) {
          pin = String(pin + key);
          pin_ekran = String(pin_ekran + key);
          timer_ukrycia_pinu = millis();
          if (key == '*') {
            pin = String("");
            pin_ekran = pin;
            lcd.clear();
            while (i == 1) {
              timer_wyjscia = millis();
              i = 0;
            }

            if (millis() - timer_wyjscia > 4000 && millis() - timer_wyjscia < 6000) {
              tryb = 0;
              i = 1;
              lcd.clear();
            }
            if (millis() - timer_wyjscia > 6000) i = 1;
          }
        }

        lcd.noCursor();
        lcd.setCursor(0, 0);
        lcd.print("Wpisz pin :");
        lcd.setCursor(0, 1);
        lcd.print(pin_ekran);
        lcd.cursor();

        if (millis() - timer_ukrycia_pinu > 400) {
          pin_ekran.replace('0', '*');
          pin_ekran.replace('1', '*');
          pin_ekran.replace('2', '*');
          pin_ekran.replace('3', '*');
          pin_ekran.replace('4', '*');
          pin_ekran.replace('5', '*');
          pin_ekran.replace('6', '*');
          pin_ekran.replace('7', '*');
          pin_ekran.replace('8', '*');
          pin_ekran.replace('9', '*');
          lcd.noCursor();
          lcd.setCursor(0, 1);
          lcd.print(pin_ekran);
          lcd.cursor();
        }
        if ( PIN == pin) {
          x = 2;
          delay(2000);
          lcd.clear();
        }
      }


      /**************************************77WPISYWANIE NOWEGO KODU77******************************************/
      if (x == 2) {
        key = keypad.getKey();
        if (key) {
          nowy_kod = String(nowy_kod + key);
          nowy_kod_ekran = String(nowy_kod_ekran + key);
          timer_ukrycia_nowego_kodu = millis();
          if (key == '*') {
            nowy_kod = String("");
            nowy_kod_ekran = nowy_kod;
            lcd.clear();
            while (i == 1) {
              timer_wyjscia = millis();
              i = 0;
            }

            if (millis() - timer_wyjscia > 4000 && millis() - timer_wyjscia < 6000) {
              tryb = 0;
              i = 1;
              lcd.clear();
            }
            if (millis() - timer_wyjscia > 6000) i = 1;
          }

          if (key == '#') {
            KOD = nowy_kod;
            x = 3;
          }
        }

        lcd.noCursor();
        lcd.setCursor(0, 0);
        lcd.print("Wpisz nowy kod :");
        lcd.setCursor(0, 1);
        lcd.print(nowy_kod_ekran);
        lcd.cursor();

        if (millis() - timer_ukrycia_nowego_kodu > 400) {
          nowy_kod_ekran.replace('0', '*');
          nowy_kod_ekran.replace('1', '*');
          nowy_kod_ekran.replace('2', '*');
          nowy_kod_ekran.replace('3', '*');
          nowy_kod_ekran.replace('4', '*');
          nowy_kod_ekran.replace('5', '*');
          nowy_kod_ekran.replace('6', '*');
          nowy_kod_ekran.replace('7', '*');
          nowy_kod_ekran.replace('8', '*');
          nowy_kod_ekran.replace('9', '*');
        }
      }

      if ( x == 3) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Zmieniono kod!");
        tryb = 0;
        kod = String("");
        kod_ekran = String("");
        delay(2000);
        lcd.clear();
      }
    }

    //////////////////////////////////////////88UZBRAJANIE ALARMU88///////////////////////
    if (tryb == 2) {
      key = keypad.getKey();
      if (key) {
        kod = String(kod + key);
        kod_ekran = String(kod_ekran + key);
        timer_ukrycia_kodu = millis();
        if (key == '*') {
          kod = String("");
          kod_ekran = kod;
          lcd.clear();
        }

      }
      lcd.noCursor();
      lcd.setCursor(0, 0);
      lcd.print("Wpisz kod :");
      lcd.setCursor(0, 1);
      lcd.print(kod_ekran);
      lcd.cursor();

      if (millis() - timer_ukrycia_kodu > 400) {
        kod_ekran.replace('0', '*');
        kod_ekran.replace('1', '*');
        kod_ekran.replace('2', '*');
        kod_ekran.replace('3', '*');
        kod_ekran.replace('4', '*');
        kod_ekran.replace('5', '*');
        kod_ekran.replace('6', '*');
        kod_ekran.replace('7', '*');
        kod_ekran.replace('8', '*');
        kod_ekran.replace('9', '*');
        lcd.noCursor();

        lcd.setCursor(0, 1);
        lcd.print(kod_ekran);
        lcd.cursor();
      }

      if (kod == KOD) {
        while (flagax) {
          flaga_uzbrojenia = 1;
          flagax = 0;
        }
        while (flaga_uzbrojenia == 1) {
          kod = String("");
          kod_ekran = String("");
          lcd.clear();
          lcd.print("Uzbrojono alarm!");
          delay(2000);
          flaga_uzbrojenia = 2;
          flagax = false;
          i = 1;
          lcd.clear();
        }
        while (flaga_uzbrojenia == 2) {

          lcd.noCursor();
          lcd.setCursor(0, 0);
          lcd.print("Czas do wyjscia:");
          lcd.setCursor(0, 1);
          if (timer_opuszczenia2 >= 60) {
            lcd.print(minuty);
          } else
            lcd.print("0");
          lcd.setCursor(1, 1);
          lcd.print(":");
          if (sekundy >= 10) {
            lcd.setCursor(2, 1);
          }
          else {
            lcd.setCursor(2, 1);
            lcd.print("0");
            lcd.setCursor(3, 1);
          }
          lcd.print(sekundy);

          while (i == 1) {
            timer_opuszczenia = millis();
            i = 0;
          }

          timer_opuszczenia2 = (600 - ((millis() - timer_opuszczenia) / 1000));
          minuty = floor(timer_opuszczenia2 / 60);
          sekundy = timer_opuszczenia2 - (minuty * 60);
//Serial.print("Licznik czasu");
//Serial.println(timer_opuszczenia2);

          if (timer_opuszczenia2 == 0) {
            kod_prawidlowy = false;
            alarm_uzbrojony = true;
            tryb = 0;
            flaga_uzbrojenia = 0;
            i = 1;
            j = 1;
            flagax = true;
            digitalWrite(backlight,LOW);
            lcd.cursor();
            lcd.clear();
           
          }
        }

      }
      delay(10);
    }
  }
}






