#include <SoftwareSerial.h>
SoftwareSerial pH(35, 34); // RX, TX

//---------------- Library -----------------
#include <Arduino.h>
#include <analogWrite.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ThingSpeak.h>
#include <WiFi.h>

// ------- Set alamat I2C LCD 0x27 ---------
LiquidCrystal_I2C lcd(0x27, 16, 2);

//--------------- PID pH -------------------
double Kp_pH = 15;
double Ki_pH = 9.9;
double Kd_pH = 5.68125;
float setpoint_pH = 7.5;
float ph_max = 7.8;
float ph_min = 7.2;
float error_basa;
float error_asam;
float integral_error_basa;
float integral_error_asam;
float derivatif_error_basa;
float derivatif_error_asam;
float last_error_basa = 0;
float last_error_asam = 0;
float PWM_basa ;
float PWM_asam ;

//--------------- PID Suhu ---------------
double Kp_suhu = 81;
double Ki_suhu = 50.625;
double Kd_suhu = 0;
float error_suhu_min;
float error_suhu_max;
float integral_error_min;
float integral_error_max;
float derivatif_error_min;
float derivatif_error_max;
float last_error_min = 0;
float last_error_max = 0;
float PWM_min;
float PWM_max;

//------------ THINGSPEAK SETTING ---------------
#define SECRET_SSID "hari bulusan 1"    // Ganti dengan Username internet yang tersedia
#define SECRET_PASS "larasati"  // Ganti dengan Password internet yang tersedia
#define SECRET_CH_ID 1657788     // Ganti dengan channel number Thingspeak
#define SECRET_WRITE_APIKEY "1M7H93JK6NRWHNHX"   // Ganti dengan API Write key Thingspeak 

char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS;  
int keyIndex = 0;
unsigned long Channel = SECRET_CH_ID;   // Channel ID Write
const char * WriteAPIKey = SECRET_WRITE_APIKEY; // Write API KEY
const char * ReadAPIKey = "2Q27A6GSP2JCC8A5"; // Read API Key

unsigned int F1 = 1;  
unsigned int F2 = 2;  
unsigned int F3 = 3;   
unsigned int F4 = 4;  
unsigned int F5 = 5;  
unsigned int F6 = 6;  

WiFiClient client; 

//Deklarasi variable
String status_pH = "........";
String status_suhu = ".......";
String pump_pH = ".......";
String pump_suhu = ".......";


//--------------- PH SETTING ------------------

#define IN1 26 //26 deklarasi pin IN1 pump asam
#define IN2 25 //25 deklarasi pin IN2 pump asam
#define IN3 33 //33 deklarasi pin IN3 pump basa
#define IN4 32 //32 deklarasi pin IN4 pump basa
#define ENA 15 //15 deklarasi pin ENA pump asam
#define ENB 2  //2  deklarasi pin ENB pump basa


//--------------- SUHU SETTING ----------------
#define ONE_WIRE_BUS  18
#define IN5  27
#define IN6  14 
#define IN7  12 
#define IN8  13
OneWire oneWire (ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float suhu = 0.0;
float setpoint_suhu = 30;
float suhu_min = 28;


//----------------- MILIS ----------------------
#define printInterval 800
unsigned long Waktuawalwrite = 0 ; // startcounting timer write
unsigned long intervalwrite = 800 ; // looping timer write
unsigned long Counterwaktu ;




void setup() {
  //------------------- NETWORK ---------------------
  Serial.begin(9600);
  pH.begin(9600);
  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client);
  
  //--------------------- LCD------------------------
  lcd.begin(); //Inisialisasi LCD
  lcd.backlight(); //Menyalakan lampu latar LCD
  lcd.clear(); //Clear LCD

  //----------------- PH SETTING --------------------
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  //------------- TEMPERATURE SETTING ---------------
  sensors.begin();
  pinMode (ONE_WIRE_BUS, INPUT);
  pinMode (IN5, OUTPUT);
  pinMode (IN6, OUTPUT);
  pinMode (IN7, OUTPUT);
  pinMode (IN8, OUTPUT);
  Counterwaktu = millis();
}



void loop() {
  
  analogWriteResolution (10);
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
 
  //------------------- NETWORK ---------------------
  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  //Mencoba untuk menghubungkan ke internet
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  
  //--------------- PEMBACAAN SENSOR PH -----------------
  String pHsensor = "";
  while(pH.available() > 0)
  {
    pHsensor += char(pH.read());
  }

  float pHValue = pHsensor.toFloat();

  
  //--------------- PERHITUNGAN SENSOR SUHU -----------------
 
  sensors.setResolution(12);
  sensors.requestTemperatures();
  float suhu = sensors.getTempCByIndex(0);


  
  //-------------------- PID PUMP BASA ----------------------
  
  Counterwaktu = millis();
  unsigned long elapsedTime = (double)(Counterwaktu - Waktuawalwrite);
  
  error_basa = setpoint_pH - pHValue ; 
  integral_error_basa += error_basa * elapsedTime ;
  derivatif_error_basa = (error_basa - last_error_basa) / elapsedTime ;
  last_error_basa = error_basa ;

  PWM_basa = (Kp_pH * error_basa) + (Ki_pH * integral_error_basa) + (Kd_pH * derivatif_error_basa);

  if (PWM_basa <= 0)
  {
    PWM_basa = 0 ;
  }
  if (PWM_basa >= 225)
  {
    PWM_basa = 225 ;
  }
  else{}
  
 

  //-------------------- PID PUMP ASAM ----------------------

  error_asam = pHValue - setpoint_pH; 
  integral_error_asam += error_asam * elapsedTime ;
  derivatif_error_asam = (error_asam - last_error_asam) / elapsedTime ;
  last_error_asam = error_asam ;

  PWM_asam = (Kp_pH * error_asam) + (Ki_pH * integral_error_asam) + (Kd_pH * derivatif_error_asam);

  if (PWM_asam <= 0)
  {
    PWM_asam = 0 ;
  }
  if (PWM_asam >= 225)
  {
    PWM_asam = 225 ;
  }
  else{}


  
  //-------------------- PID SUHU MIN ----------------------

  error_suhu_min = setpoint_suhu - suhu;
  integral_error_min += error_suhu_min * elapsedTime;
  derivatif_error_min = (error_suhu_min - last_error_min) / elapsedTime;
  last_error_min = error_suhu_min;

  PWM_min = (Kp_suhu * error_suhu_min) + (Ki_suhu * integral_error_min) + (Kd_suhu * derivatif_error_min);
  
  if (PWM_min <= 0)
  {
    PWM_min = 0 ;
  }
  if (PWM_min >= 225)
  {
    PWM_min = 225 ;
  }
  else{}


  //-------------------- PID SUHU MAX ----------------------

  error_suhu_max = suhu - setpoint_suhu;
  integral_error_max += error_suhu_max * elapsedTime;
  derivatif_error_max = (error_suhu_max - last_error_max) / elapsedTime;
  last_error_max = error_suhu_max;

  PWM_max = (Kp_suhu * error_suhu_max) + (Ki_suhu * integral_error_max) + (Kd_suhu * derivatif_error_max);
  
  if (PWM_max <= 0)
  {
    PWM_max = 0 ;
  }
  if (PWM_max >= 225)
  {
    PWM_max = 225 ;
  }
  else{}
  delay (1000);


  
  //------------- WRITE SENSOR TO THINGSPEAK ---------------
  
  if(Counterwaktu - Waktuawalwrite > intervalwrite)
  {
    Waktuawalwrite = Counterwaktu ;
    ThingSpeak.setField(F1, pHValue);        
    ThingSpeak.setField(F2, suhu);
    ThingSpeak.setField(F3, status_pH);       
    ThingSpeak.setField(F4, status_suhu);
    ThingSpeak.setField(F5, pump_pH ); 
    ThingSpeak.setField(F6, pump_suhu );
    ThingSpeak.writeFields(Channel, WriteAPIKey);

    
  
  //---------------- DECLARE KEADAAN -----------------
  if (millis() - Waktuawalwrite > intervalwrite)
  { 
    if (pHValue > setpoint_pH)
    {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      analogWrite (ENA, PWM_asam);
      pump_pH = "pH Down"; //akan ditampilkan di aplikasi, F3 thingspeak
    }

    else
    {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      analogWrite (ENA, 0);
    }
    
    if (pHValue < setpoint_pH)
    {
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite (ENB, PWM_basa);
      pump_pH = "pH Up"; //akan ditampilkan di aplikasi, F3 thingspeak
    }
    
    else
    {
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      analogWrite (ENB, 0);
    }

    
  //----------Deklarasi Keadaan Suhu----------   
    if (suhu < setpoint_suhu)
    {
    analogWrite(IN5, PWM_min);
    analogWrite(IN6, 0);
    analogWrite(IN7, PWM_min);
    analogWrite(IN8, 0);
    pump_suhu = "ON";
    
    }

    else

    if (suhu > setpoint_suhu)
    {
    analogWrite(IN5, PWM_max);
    analogWrite(IN6, 0);
    analogWrite(IN7, PWM_max);
    analogWrite(IN8, 0);
    pump_suhu = "ON";
    
    }

    else
    {
    analogWrite(IN5, 0);
    analogWrite(IN6, 0);
    analogWrite(IN7, 0);
    analogWrite(IN8, 0);
    pump_suhu = "OFF";
    
    }
  }

  if(millis() - printTime > printInterval)   //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
  {
    lcd.setCursor(0, 0);
    lcd.print("PH : ");
    lcd.print(pHValue);
    lcd.setCursor(0, 1);
    lcd.print("Suhu : ");
    lcd.print(suhu);
    
    
    //Serial.print("Voltage:");
    //Serial.print(voltage);
    //Serial.print(setpoint);
    //Serial.print("pH : ");
    Serial.print(setpoint_suhu);
    Serial.print(" ");
    Serial.print(suhu);
    Serial.print(" ");
    Serial.print(setpoint_pH);
    Serial.print(" ");
    Serial.println(pHValue);
    /*Serial.print("PWM basa : ");
    Serial.print(PWM_basa);
    Serial.print("           PWM asam : ");
    Serial.println(PWM_asam);
    Serial.print("PWM min : ");
    Serial.print(PWM_min);
    Serial.print("           PWM max : ");
    Serial.println(PWM_max);*/
    delay (1000);
    
    printTime=millis();
  }
 }
}
