#include <MsTimer2.h>     //定時器庫的標頭檔案
#include <TimerOne.h>


int tick = 0; //計數值
float adc0_value;
float memo_1=0;
int sys=0;
int dia=0;
int Pulse=0;
int Pulse_memo=0;
int rate = 0;
int LOOP = 0;
int count = 0;
int time_out = 0;
int start = 3;
int state = 0;
int real;
int def = 0;
float det = 0;
int LED_ = 13;
int inChar ;
int x = 0;
int y = 0;
int pump = 0;
float comp = 0.6 ;    //偵測靈敏度 0.5~0.9
#define puls_out 7
#define pump_sw 12
#define air 2
#define sa 10

byte sd[6] = {0xaa, 0x11, 0x00, 0x00, 0x02, 0x03}; //收縮壓
byte pd[6] = {0xaa, 0x33, 0x00, 0x00, 0x02, 0x03}; //舒張壓
byte hr[6] = {0xaa, 0x55, 0x00, 0x00, 0x02, 0x03}; //心律
byte current[6] = {0xaa, 0x77, 0x00, 0x00, 0x02, 0x03}; //當前壓力

float vol = 32000/4095;  //40Kpa壓力Sensor 將AD值轉換為壓力值
const long interval = 100 ; //  100 microseconds 
//中斷服務程式
void TimerOne(){
  int SwState = digitalRead(pump_sw);
  if (state == 1){                     //開始充氣
    digitalWrite(air,HIGH);
    analogWrite(sa,1600);  //1600  
    start = 0;
  } 
  else if (state == 2){               //洩氣
    analogWrite(sa,0);
    digitalWrite(air,LOW);
  }
  else {
    digitalWrite(air,LOW);
    if (start != 3){
        x++;
        pump = pump +real;
        if (x >= 5){
          pump = pump / 5;
          if ( pump >= 150){
            analogWrite(sa,1250);  //1250  調整洩氣速度
            x = 0;
            pump = 0;
          }
          else if ( pump >= 140){
            analogWrite(sa,1200);  //1200調整洩氣速度
            x = 0;
            pump = 0;
          }
          else if ( pump >= 130){
            analogWrite(sa,1150);  //1150調整洩氣速度
            x = 0;
            pump = 0;
          }
          else if ( pump >= 120){
            analogWrite(sa,1070);   //1070調整洩氣速度
            x = 0;
            pump = 0;
          }
          else if ( pump >= 90){
            analogWrite(sa,980);   //980調整洩氣速度
            x = 0;
            pump = 0;
          }
          else if ( pump >= 80){
            analogWrite(sa,760);  //760調整洩氣速度
            x = 0;   
            pump = 0;
          }
          else {
            x = 0;
            pump = 0;
            comp = 0.9;
          }
        }
    }
  }
}
//中斷服務程式2
void onTimer()
{
  rate++;
  if (start == 1){
      time_out++;      
  } 
}

void serialEvent() {
  while (Serial.available()) {
    // Read data from USB OTG
    inChar = Serial.read();   //(char)Serial.read(); 
    if (inChar == 0x61){
      state= 1;
      def = 145;
    } 
    else if (inChar == 0x62){
      state= 1;
      def = 166;
    }
    else if (inChar == 0x63){
      state= 1;
      def = 185;
    }
    else if (inChar == 0x64){
      state = 2;
    }
    else {
      state= 2;
    }
  }
}
void setup()
{
  Serial.begin(9600); //初始化串列埠
  
  MsTimer2::set(10, onTimer); //設定中斷，每10ms進入一次中斷服務程式 onTimer()
  MsTimer2::start(); //開始計時
  Timer1.initialize( interval ); // 初始化,  interval 以 micro sec 為單位
  Timer1.attachInterrupt( TimerOne ); // attach the service routine here

  pinMode(LED_, OUTPUT);
  pinMode(puls_out, OUTPUT);
  pinMode(pump_sw, INPUT_PULLUP);
  pinMode(air, OUTPUT);
  //pinMode(sa, OUTPUT);
  analogWrite(sa,0); 
  analogReference(INTERNAL4V096);  // set analog reference to internal 4.096V
}

void loop(){
  adc0_value = analogRead(A0)* vol/133; //將壓力值轉為mmHg，1Kpa=7.51mmHg
  real= adc0_value;
  if (LOOP >5 ){             //即時壓力送出時間
    current[3] = real;       //for app
    Serial.write(current, sizeof(current));   //for app
    LOOP = 0;
  }
  LOOP++;   //延遲即時壓力送出時間
  
  if (real >= def ){
    delay (500);
    state =0;
  }
  if (memo_1 >= adc0_value){
    memo_1 = adc0_value;
    if (start == 1){
      if ((time_out >= 300)&&(count > 1)){
        dia = adc0_value;
        Pulse = (60000/(Pulse/count*13.5));  //計算心率
        start = 2;      //量測結束
        out_put();
      }
    }
  }
  else {  
    if ((adc0_value-memo_1)>=comp){    //壓力差值(靈敏度調整)
      time_out = 0;
      if (rate > 150){
        if (start  == 0 ){
          sys = adc0_value;
          start = 1;            //開始量測
          time_out = 0;
          count = 0;
          rate = 0;
          Pulse = 0;
        } 
      }
      else{
        if (rate >=30){
          if ((rate <= (Pulse_memo +30)) || (rate >= (Pulse_memo -30))){
            Pulse_memo = rate;
            Pulse = Pulse+rate;
            count++;
          }
          else {
            if (count <=2 ){
              count = 0;
              Pulse_memo = rate;
            }
          }
        }       
       }
         digitalWrite(LED_, HIGH);   // turn the LED on (HIGH is the voltage level)
       delay(100);                       
       digitalWrite(LED_, LOW);    // turn the LED off by making the voltage LOW
       delay(100);       
       memo_1 = adc0_value;
       rate=0;
     }
  }
  delay(10);
}

//===================for app ============================
void out_put(){
  if (count > 3){ 
    delay(50); //
    sd[3] = sys; //(舒張壓)
    Serial.write(sd, sizeof(sd));
    delay(50); //
    pd[3] = dia;   //收縮壓
    Serial.write(pd, sizeof(pd));
    delay(50); //
    hr[3] = Pulse;  //心率
    Serial.write(hr, sizeof(hr));
    delay(50); //
    current[3] = adc0_value;  //即時壓力
    Serial.write(current, sizeof(current));
    delay(30); //
  }
  start = 3;
  count = 0;
  time_out = 0;
  analogWrite(sa,0);
  inChar = "n";
}
//==============================================
//============================================
