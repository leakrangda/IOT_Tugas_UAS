#define BLYNK_TEMPLATE_ID "TMPL2MsXaDsh"
#define BLYNK_DEVICE_NAME "TugasUas"
#define BLYNK_AUTH_TOKEN "suMmLPmP44fvyB9jO4zQIDsJ6Cm3B_RT"

#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "R. ASET";
char pass[] = "qwertyuiop";

int dataClk = 26;
int latchClk = 25;
int shiftClr = 27;
int ser = 32;
int oe = 33;
int an1 = 2;
int an2 = 15;
byte d = 0;
bool tr=true;
bool setted=false;
float tdl=1320;
float harga=0.0;

PZEM004Tv30 pzem(&Serial2, 16, 17);
BlynkTimer timer;
void setup()
{
  pinMode(dataClk, OUTPUT);
  pinMode(latchClk, OUTPUT);
  pinMode(shiftClr, OUTPUT);
  pinMode(oe, OUTPUT);
  pinMode(ser, OUTPUT);
  pinMode(an1, INPUT);
  pinMode(an2, INPUT);
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sender);
}
void loop()
{
  if(Serial.available()>=0){
    String cmd(Serial.readStringUntil('\n'));
    if(cmd == "info"){
      Serial.print("voltage = ");
      Serial.println(pzem.voltage());
      Serial.print("ampere = ");
      Serial.println(pzem.current());
      Serial.print("pfactor = ");
      Serial.println(pzem.pf());
      Serial.print("power=");
      Serial.println(pzem.power());
      Serial.print("energy=");
      Serial.println(pzem.energy());
      Serial.print("frequency=");
      Serial.println(pzem.frequency());
    }else if(cmd == "off"){
      d = 0xff;
      setRelay(d);
    }else if(cmd == "on"){
      d = 0x0;
      setRelay(d); 
    }else if(cmd == "readAnalaog"){
      readCurrent(an1, 185, 2.5);
      readCurrent(an2, 100, 2.5);
    }
  }
  if(tr && !setted){
    setRelay(d);
    setted=false;
  }
  Blynk.run();
  timer.run();
}
int setRelay(byte b){
  digitalWrite(oe, LOW);
  digitalWrite(shiftClr, HIGH);
  digitalWrite(latchClk, LOW);
  shiftOut(ser, dataClk, MSBFIRST, b);
  digitalWrite(latchClk, HIGH);
  digitalWrite(shiftClr, LOW);
  return 0;
}
float getTrueVoltage(int pin, int siz){
  //menghitung arus ac secara true rms
  float result=0.0;
  float window[siz];

  for(int count=0; count < siz; count++){
    float hasil = map(analogRead(pin), 0, 4095, 0, 3.3);
    window[count] = hasil*hasil; 
  }
  for(int count=0; count < siz; count++){
    result=result + window[count];
  }
  return sqrt(result/siz);
}
float readCurrent(int pin, float mAmp, float offset){
  Serial.print("result:");
  
  float volt = getVPP(pin);
  float VRMS = (volt/2.0) *0.707;
  float cur = ((VRMS * 1000)/mAmp);
  Serial.println(cur);
  return cur;
}
float getVPP(int sensorIn)
{
  float result;
  int readValue;                // value read from the sensor
  int maxValue = 0;             // store max value here
  int minValue = 4096;          // store min value here ESP32 ADC resolution
  
   uint32_t start_time = millis();
   while((millis()-start_time) < 200) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minimum sensor value*/
           minValue = readValue;
       }
   }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 3.3)/4096.0; //ESP32 ADC resolution 4096
      
   return result;
 }

 void sender(){
  Blynk.virtualWrite(V0, pzem.energy());
  Blynk.virtualWrite(V1, pzem.voltage());
  Blynk.virtualWrite(V2, pzem.current());
  harga=harga + (pzem.energy()/1000.0) * (tdl/3600.0);
  Blynk.virtualWrite(V3, harga);
  Blynk.virtualWrite(V6, readCurrent(an1, 185, 2.5));
  Blynk.virtualWrite(V7, readCurrent(an2, 100, 2.5));
  Serial.println(digitalRead(V4));
  return;
 }
BLYNK_WRITE(V5){
  int value = param.asInt();

  if(value)
    d = d | 2;
  else
    d = d ^ 2;
}
BLYNK_WRITE(V4){
  int value = param.asInt();

  if(value)
    d = d | 1;
  else
    d = d ^ 1;
}
