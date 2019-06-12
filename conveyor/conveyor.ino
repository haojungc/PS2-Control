#include <SoftwareSerial.h>
#include "DCmotor.h"
#include <Wire.h>             //存取接在 TWI/I2C Bus 上的裝置或感測器
MOTOR conveyor;
SoftwareSerial BT(6, 7);  //定義 PIN7 及 PIN6 分別為 RX 及 TX 腳位

char num ;
void conveyorControl();


void setup() {
  conveyor.setPin(10, 11);
  Serial.begin(9600);           //Arduino起始鮑率：9600
  Serial.println("Enter AT commands:");
  BT.begin(9600);           //藍牙鮑率：38400 注意！每個藍牙晶片的鮑率都不太一樣，請務必確認

}


void loop() {
  int inSize;
  char input;

  for (;;) {
    if ((inSize = (BT.available())) > 0) { //讀取藍牙訊息
      //Serial.print("size = ");
      //Serial.println(inSize);
      //Serial.print("input = ");
      //Serial.println(input = (char)BT.read());
      num = (char)BT.read();
      //Serial.println((char)num);
    }
    conveyorControl();
  }
delay(30);
}

#define A 0
#define B 1
#define q 2
void conveyorControl() {
  static byte mode = q;
  if (num == 'A') {
    mode = A;
  }
  else if (num == 'B') {
    mode = B;
  }
  else if (num == 'q') {
    mode = q;
  }

  if(mode == A){
    Serial.println("向前");
    conveyor.spin(255, CW);
  }
  else if(mode == B){
    Serial.println("向後");
    conveyor.spin(255, CCW);
  }
  else{
    Serial.println("停");
    conveyor.spin(0);
  }
}

/*

  //自動模式
  for(;;){
   if(BTSerial.available()){
    num= BTSerial.read();
   }
   }
   Serial.println("第二步驟結束");
  }


       if(num == 'n')
          {
            Serial.println(" 我走了 " );
            num = 0;
          break;
          }
       else
          {
          switch (num){

         //夾爪開度
            case 'A':
            Serial.print("合爪");
            close1();

            break;

            case 'B':
            Serial.print("開爪");
            open1();
            break;

     }
  }
  int close1(){  //按下button後，傳入正轉的指令;放開button後，傳入停止的訊號並記錄正轉了幾圈

      for(;;){
        Serial.println("夾緊");
          myservo.write(180);
           delay(500);
        if(BTSerial.read()== 'q'){

          myservo.write(90);
          Serial.println("停");
          break;}
        }
        }


  int open1(){ //按下button後，傳入反轉的指令;放開button後，傳入停止的訊號並記錄反轉了幾圈
   digitalWrite(6,HIGH);
  for(;;){

        Serial.println("打開");
          myservo.write(0);
           delay(500);
        if(BTSerial.read()== 'q'){
          digitalWrite(6,LOW);
          myservo.write(90);
          Serial.println("停");
          break;}
        }
         }

  }
  int stepzero(){
  }*/
