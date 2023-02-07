#define voutPin 0
#define readPin 2
#define xckPin 3
#define resetPin 4
#define loadPin 5
#define sinPin 6
#define startPin 7

#define reg0 0x00
#define reg1 0x0F //ゲイン
#define reg2 0x05 //C1 露光時間
#define reg3 0x00 //C0 露光時間
#define reg4 0x01
#define reg5 0x00
#define reg6 0x01
#define reg7 0x07

#define Xck_H digitalWrite(xckPin,HIGH)
#define Xck_L digitalWrite(xckPin,LOW)

unsigned char data[256];

// the setup routine runs once when you press reset:
void setup() {
// initialize serial communication at 9600 bits per second:
Serial.begin(9600);

pinMode(readPin, INPUT);
pinMode(xckPin, OUTPUT);
pinMode(resetPin, OUTPUT);
pinMode(loadPin, OUTPUT);
pinMode(sinPin, OUTPUT);
pinMode(startPin, OUTPUT);

}

// the loop routine runs over and over again forever:
void loop() {
char buf;
int adcnt;
int datacnt;
int i;

digitalWrite(resetPin,LOW);//RESET -> L
Xck_H;
Xck_L;
Xck_H;
Xck_L;
digitalWrite(resetPin,HIGH);//RESET -> H リセット解除
Xck_H;
Xck_L;

//レジスタ設定
setReg(2,reg2);
setReg(3,reg3);
setReg(1,reg1);
setReg(0,reg0);
setReg(4,reg4);
setReg(5,reg5);
setReg(6,reg6);
setReg(7,reg7);

Xck_H;
Xck_L;
digitalWrite(startPin,HIGH);//スタートH　カメラスタート
Xck_H;
digitalWrite(startPin,LOW);//スタートL
Xck_L;
Xck_H;

while(digitalRead(readPin)==LOW){//READシグナル待
Xck_L;
Xck_H;
}

adcnt=0;
datacnt=0;

//今回は８列８行毎に32×32画素を取得する
while(datacnt<255){
if((adcnt&0x0070)==0x0000){//８列中１列データ取得
data[datacnt]=(analogRead(voutPin)/4); //ADの最大値1024を8bitに圧縮
datacnt=datacnt+1;
}
adcnt=adcnt+1;
Xck_L; Xck_H;
Xck_L; Xck_H;
Xck_L; Xck_H;
Xck_L; Xck_H;
Xck_L; Xck_H;
Xck_L; Xck_H;
Xck_L; Xck_H;
Xck_L; Xck_H;
}

//シリアルモニタに画像データを表示する
for(i=0;i<256;i++){
if(i%16==0){Serial.println(" ");}
if(data[i]<100){Serial.print(" ");}
if(data[i]<10){Serial.print(" ");}
Serial.print(data[i], DEC);
Serial.print(" ");
}
Serial.println(" ");

while(1){}
}

void setReg( unsigned char adr, unsigned char data )
{
//アドレス転送(3bit)
if((adr&0x04)==0x04){digitalWrite(sinPin,HIGH);}else{digitalWrite(sinPin,LOW);}
Xck_H; Xck_L;
if((adr&0x02)==0x02){digitalWrite(sinPin,HIGH);}else{digitalWrite(sinPin,LOW);}
Xck_H; Xck_L;
if((adr&0x01)==0x01){digitalWrite(sinPin,HIGH);}else{digitalWrite(sinPin,LOW);}
Xck_H; Xck_L;

//データ転送(8bit)
if((data&0x80)==0x80){digitalWrite(sinPin,HIGH);}else{digitalWrite(sinPin,LOW);}
Xck_H; Xck_L;
if((data&0x40)==0x40){digitalWrite(sinPin,HIGH);}else{digitalWrite(sinPin,LOW);}
Xck_H; Xck_L;
if((data&0x20)==0x20){digitalWrite(sinPin,HIGH);}else{digitalWrite(sinPin,LOW);}
Xck_H; Xck_L;
if((data&0x10)==0x10){digitalWrite(sinPin,HIGH);}else{digitalWrite(sinPin,LOW);}
Xck_H; Xck_L;
if((data&0x08)==0x08){digitalWrite(sinPin,HIGH);}else{digitalWrite(sinPin,LOW);}
Xck_H; Xck_L;
if((data&0x04)==0x04){digitalWrite(sinPin,HIGH);}else{digitalWrite(sinPin,LOW);}
Xck_H; Xck_L;
if((data&0x02)==0x02){digitalWrite(sinPin,HIGH);}else{digitalWrite(sinPin,LOW);}
Xck_H; Xck_L;
if((data&0x01)==0x01){digitalWrite(sinPin,HIGH);}else{digitalWrite(sinPin,LOW);}
Xck_H;
digitalWrite(loadPin,HIGH); //LOADピン→H
digitalWrite(sinPin,LOW);
Xck_L;
digitalWrite(loadPin,LOW); //LOADピン→L
}
