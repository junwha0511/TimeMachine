#include <DS1302.h> //RTC 모듈 포함
//RTC 핀 정의
int sckPin =  A4; 
int IOPin = A3;  
int rstPin = A2;
//버튼 핀 정의
int realButton = A0;    
int fakeButton = A1; 
//LED 핀 정의
int realLED = A6;
int fakeLED = A5;
//RTC 클래스 정의
DS1302 rtc(rstPin, IOPin, sckPin);
//Time 클래스 정의
Time t;

boolean fakeFlag = false; //기본모드: 현재시간모드
int randNum = random(3); //0,1,2 중 랜덤으로 수를 저장

int position_pin[] = {1,2,3,4};               //4자리 결정 핀
int segment_pin[] = {5,6,7,8,9,10,11,12};     //세그먼트 제어 핀
const int delayTime = 5;                      //일시정지 시간

//0 ~ 9를 표현하는 세그먼트 값
byte data[] = {0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE4, 0xFE, 0xE6};

void setup()
{
  //버튼 핀 초기화
  pinMode(realButton, INPUT);
  pinMode(fakeButton, INPUT);
  //LED 핀 초기화
  pinMode(realLED, OUTPUT);
  pinMode(fakeLED, OUTPUT);
  
  for(int i = 0; i < 4; i++) {
     pinMode(position_pin[i], OUTPUT);
  }

  //세그먼트 제어 핀 출력용으로 설정
  for(int i = 0; i < 8; i++) {
    pinMode(segment_pin[i], OUTPUT);
  }

  //halt 플래그 초기화
  rtc.halt(false);
  //RTC 모듈 잠금 해제
  rtc.writeProtect(false);
  //시리얼 통신 시작(코딩용)
  Serial.begin(9600);
  //난수 시드 입력
  randomSeed(A0);
//초기 시간 설정 코드(시간 안 맞을 때 사용할 것!)
//  rtc.setDOW(SATURDAY);
//  rtc.setTime(19,07,5 );
//  rtc.setDate(13,10,2018); 
}
 
void loop()
{
  if(digitalRead(realButton) == HIGH){ //현재 시간을 불러오는 버튼이 눌렸을 때
     fakeFlag=false; //현재 시간 모드로 설정
     t=getRealTime(); //현재 시간을 Time 클래스에 저장
     digitalWrite(realLED, HIGH);
     digitalWrite(fakeLED, LOW);
     
  }else if(digitalRead(fakeButton) == HIGH){ //가짜 시간을 불러오는 버튼이 눌렸을 때
     fakeFlag=true; //가짜 시간 모드로 설정
     randNum = random(3); //0,1,2 중 랜덤으로 수를 저장
     t=getRandomTime(randNum); //가짜 시간을 Time 클래스에 저장
     digitalWrite(realLED, LOW);
     digitalWrite(fakeLED, HIGH);
  }else{ //두 버튼 모두 눌리지 않았을 때
    if(fakeFlag){ //가짜 시간으로 동작하고 있을 때
      if(getRealTime().min==0){ //1시간 마다 랜덤으로 10분 빨리, 10분 느리게, 현재 시간 중 정함
        randNum = random(3); //0,1,2 중 랜덤으로 수를 저장
        t=getRandomTime(randNum); //randNum을 기준으로 시간 가져옴
      }else{
        t=getRandomTime(randNum); //randNum을 기준으로 시간 가져옴
      }
    }else if(not fakeFlag){ //현재 시간으로 동작하고 있을 때
       t=getRealTime(); //현재 시간을 Time 클래스에 저장
    }
  }
  Serial.print(t.hour);
  Serial.print(":");
  Serial.println(t.min);

  if(t.hour >= 10)
  {
    show(1,t.hour/10);
    delay(delayTime);
    show(2,t.hour%10);
    delay(delayTime);
  }
  else {
    show(1,0);
    delay(delayTime);
    show(2,t.hour);
    delay(delayTime);
  }

  if(t.min >= 10)
  {
    show(3,t.min/10);
    delay(delayTime);
    show(4,t.min%10);
    delay(delayTime);
  }
  else{
    show(3,0);
    delay(delayTime);
    show(4,t.min);
    delay(delayTime);
  }
}

void show(int position, int number) {
  //4자리 중 원하는 자리 선택
  for(int i = 0; i < 4; i++) {
    if(i + 1 == position){
      digitalWrite(position_pin[i], LOW);
    } else {
      digitalWrite(position_pin[i], HIGH);
    }
  }

  //8개 세그먼트를 제어해서 원하는 숫자 출력
  for(int i = 0; i < 8; i++){
     byte segment = (data[number] & (0x01 << i)) >> i;
     if(segment == 1){
       digitalWrite(segment_pin[7 - i], HIGH);
     } else {
       digitalWrite(segment_pin[7 - i], LOW);
     }
  }
}

Time getRandomTime(int randomNum){ //랜덤으로 시간을 가져오는 메소드
  if(randNum==0){ 
     return getRealTime(); //현재 시간을 리턴
  }else if(randNum==1){
     return getEarlierTime(); //10분 빠른 시간을 리턴
  }else if(randNum==2){
     return getLaterTime(); //10분 느린 시간을 리턴
  }
}
Time getRealTime(){ //현재 시간을 가져오는 메소드
  t=rtc.getTime(); //현재 시간을 가져와서 t에 저장
  return t; //t를 리턴
}

Time getEarlierTime(){ //10분 빠른 시간을 가져오는 메소드
  t=rtc.getTime(); //현재 시간을 가져옴 
  if(t.min>=10){ //min이 10보다 크거나 같을 때는
    t.min-=10; //10을 뺀 값을 저장
  }else{ //min이 10보다 작을 때는
    t.min+=50; //50을 더한 값을 저장하고
    if(t.hour!=0){ //hour가 0이 아닐 경우
      t.hour-=1; //hour를 1 감소시킴XX
    }else if(t.hour==0){ //hour가 0일 경우
      t.hour=23; //hour가 23이 됨
    }
  }
  return t; //t를 리턴
}

Time getLaterTime(){ //10분 느린 시간을 가져오는 메소드
  t=rtc.getTime(); //현재 시간을 가져옴
  if(t.min<50){ //min이 50보다 작을 때는
    t.min+=10; //10을 더한 값을 저장
  }else{ //min이 50보다 크거나 같을 때는
    t.min-=50; //50을 빼고
    if(t.hour!=23){ //hour가 23이 아니면
      t.hour+=1; //hour를 1 증가시킴
    }else if(t.hour==23){ //hour가 23이면
      t.hour=               0; //hour가 0이 됨
    }
  }
  return t; //t를 리턴
}



