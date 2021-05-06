#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

int Trig = 8;
int Echo = 9;
int Trig2 = 11;
int Echo2 = 12;

int red = 3;
int green = 5;
int blue = 6;

unsigned long duration, duration2;
float distance, distance2;


int sen1avgdis, sen2avgdis;                   //초기에 벽까지 측정된 거리를 평균을 낸 변수
int sen1cnt=0, sen2cnt=0;                     //초기에 벽까지 측정된 거리를 모두 합한 변수
int total1, total2;                           //초기에 벽까지 측정된 거리를 카운트하여 평균이 나온 후에는 초기 측정을 끝내기 위한 변수

long sen1time=0,  sen2time=0;                 //센서에 사람이 감지된 이후 탈출 시간을 저장하는 변수
int sen1det, sen2det;                         //센서에 사람이 감지되었는지 확인 유무를 위한 변수
int lastcalcheck=0;                           //두 센서에 사람이 감지되었을때에만 카운트를 하기 위한 변수
int sen1calcheck, sen2calcheck;               //한 센서에 사람이 감지 되었을때 시간을 계산하기 위한 변수


long sen1ltdelaytime = 0,  sen2ltdelaytime = 0;
int sen1delaycnt = HIGH;                      //센서1 활성화를 결정하는 변수
int sen2delaycnt = HIGH;                      //센서2 활성화를 결정하는 변수
long sen1delaytime = 0;
long sen2delaytime = 0;

int delcnt=1;
int cnt = 0;                                  //지나간 사람들을 카운트하는 변수

unsigned int oneSecPeriod = 0;
unsigned long sec, minute, hour, date, month, year;
unsigned long daycal = 0, monthcal, datecal, leapyear;
char *day[] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
char *ampm[] = {"AM", "PM"};
int ampmcnt = 0;


void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  
  year = 2020;
  month = 12;
  date = 9;
  hour = 11;
  minute = 00;
  sec = 0;

  //윤년 일수 계산 
  for(int i = 1900; i <= year; i++){
    if((i % 4 == 0 && i % 100 != 0) || i % 400 == 0){ //윤년 계산, 4년과 400년마다, 100으로 나누어 떨어지지 않는 해일때
      leapyear++;
    }
  }

  //날짜 계산 
  for(int j = 1; j < month; j++){
    if(j == 1 || j == 3 || j == 5 || j == 7 || j == 8 || j == 10 || j == 12){
      monthcal = monthcal + 31;
    }
    else if(j == 4 || j == 6|| j == 9 || j == 11){
      monthcal = monthcal + 30;
    }
    else if(j == 2){
      monthcal = monthcal + 28;
    }
  }
  
  //날짜 계산
  datecal = date - 1;
  daycal = (year - 1900) * 365 + monthcal + datecal + leapyear;
  
  
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);

  pinMode(Trig2, OUTPUT);
  pinMode(Echo2, INPUT);

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);  

  analogWrite(5, 255);
}

void loop() {
  digitalWrite(Trig, LOW);
  digitalWrite(Echo, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  
  duration = pulseIn(Echo, HIGH);                   //pulseIn은 펄스를 읽는 기능, 마이크로 세컨드로 리턴
  distance = ((340 * duration) / 10000) / 2;        //거리=속도*시간
                                                    //거리 =  340m/s(음속)* 시간 / 10000 (cm 단위를 이용할 것이기 때문에 백만이 아닌 만으로 나눔) / 2 (왕복했기 때문에 2로 나눔)

  if(sen1cnt <= 30){                                //센서1 초기 카운트 횟수가 30 이하일경우
    total1 = total1 + distance;                     //거리 총합 계산
  
    if(sen1cnt == 30) sen1avgdis = total1 / 30;     //센서1에서 측정된 30개의 거리 데이터의 평균
    sen1cnt++;
    Serial.print("Sensor 1's average distance: ");
    Serial.println(sen1avgdis);
  }

  if(distance <= sen1avgdis - 30){                  //평균거리보다 30cm 앞에 사람이 감지됬을때(조건은 환경에 따라 유동적으로 정해야함)
    sen1det = 1;                                    //사람이 감지되었을시 1, 아래에 센서 1과 2에 모두 감지가 됬을때 연산을 하기위한 조건
    sen1calcheck = 1;                               //아래 계산 유무를 확인하기 위해 만든 변수의 값을 1로 만듬
  }
  else  sen1calcheck = sen1calcheck + 2;            //감지가 안됬을시 +2, 감지가 끝난 직후에만 연산하기 위한 장치

  if(sen1calcheck == 3 && sen1delaycnt == HIGH){    //2를 더해서 3일때만 참, 한번 조건문을 돌면 2씩 커지기 때문에 사람이 아예 감지 되지 않으면 조건문이 안돌아가게 하는 장치      
    lastcalcheck = lastcalcheck + sen1calcheck;     //아래 두 값을 비교할때 연산 유무를 가리는 변수에 +3
    sen1calcheck = 0;                               //계산 확인 변수 초기화
    sen1time = millis();                            //시간이 오래되어 데이터를 버리기 위한 시간을 측정
    sen1delaycnt = LOW;                             //센서1 불활성화
    sen1ltdelaytime = sen1time;                     //센서1 탈출시점의 시간 저장
    Serial.println("sen1 detect");                  
    Serial.print("sen1 lastcalcheck: ");
    Serial.println(lastcalcheck);
  }

  digitalWrite(Trig2, LOW);
  digitalWrite(Echo2, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig2, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig2, LOW);

  unsigned long duration2 = pulseIn(Echo2, HIGH);
  distance2 = ((340 * duration2) / 10000) / 2;
  
  if(sen2cnt <= 30){                                 //센서2 초기 카운트 횟수가 30 이하일경우
    total2 = total2 + distance2;

    if(sen2cnt == 30) sen2avgdis = total2 / 30;      //센서2에서 측정된 30개의 거리 데이터의 평균
    sen2cnt++;
    Serial.print("Sensor 2's average distance: ");
    Serial.println(sen2avgdis);
  }

  if(distance2 <= sen2avgdis - 30){                   //평균거리보다 50cm 앞에 사람이 감지됬을때(조건은 환경에 따라 유동적으로 정해야함)
    sen2det = 1;                                      //사람이 감지되었을시 1, 아래에 센서 1과 2에 모두 감지가 됬을때 연산을 하기위한 조건
    sen2calcheck = 1;                                 //아래 계산 유무를 확인하기 위해 만든 변수의 값을 1로 만듬
  }
  else  sen2calcheck = sen2calcheck + 2;              //감지가 안됬을시 +2, 감지가 끝난 직후에만 연산하기 위한 장치
  
  if(sen2calcheck == 3 && sen2delaycnt == HIGH){      //2를 더해서 3일때만 참, 한번 조건문을 돌면 2씩 커지기 때문에 사람이 아예 감지 되지 않으면 조건문이 안돌아가게 하는 장치
    lastcalcheck = lastcalcheck + sen2calcheck + 1;   //아래 두 값을 비교할때 연산 유무를 가리는 변수에 +3
    sen2calcheck = 0;                                 //계산 확인 변수 초기화
    sen2time = millis();                              //탈출 시간을 측정
    sen2delaycnt = LOW;                               //센서2 비활성화
    sen2ltdelaytime = sen2time;                       //센서2의 탈출시점의 시간 저장
    Serial.println("sen2 detect");
    Serial.print("sen2 lastcalcheck: ");
    Serial.println(lastcalcheck);
  
  }
  
  if((sen1det == 1 && sen2det == 1) && lastcalcheck == 7){    //두개의 센서에 사람이 감지되고 평균시간이 둘다 연산되었을 경우 (lastcalcheck로 한 이유: 7이 소수이기 때문에)
    if(sen1time - sen2time < 0){                              //센서1에서 감지된 탈출시간이 센서2에서 감지된 탈출시간보다 작다면
      cnt++;                                                  //사람이 안으로 들어갔으므로 인원 +1
    }
    if(sen1time - sen2time > 0){                              //센서1에서 감지된 탈출시간이 센서2에서 감지된 탈출시간보다 크다면
      if(cnt != 0){                                           //인원수가 0명이 아니면(음수로 카운터 되는 것을 막기 위함)
        cnt--;                                                //사람이 밖으로 나갔으므로 인원 -1
      }
     
    }
    
    Serial.print("sen1detecttime: ");
    Serial.println(sen1time);
    Serial.print("sen2detecttime: ");
    Serial.println(sen2time);
    sen1time = 0;                                             //센서1의 평균시간 초기화
    sen2time = 0;                                             //센서2의 평균시간 초기화
    sen1det == 0;                                             //센서1의 탐지된 상태 초기화
    sen2det == 0;                                             //센서2의 탐지된 상태 초기화
    Serial.print("Lastcalcheck: ");
    Serial.println(lastcalcheck);
    lastcalcheck = 0;                                         //계산 유무 초기화
    Serial.print("CNT: ");
    Serial.println(cnt);
  }
  
  if(lastcalcheck > 7 || lastcalcheck == 6){          //lastcalcheck가 7보다 크거나 6일경우(둘이 같이 감지했을때 7이 되는데 이 조건은 한 센서에만 두번 감지된 경우를 뜻함)
    sen1time = 0;                                     //센서1에서 감지된 시간 초기화
    sen2time = 0;                                     //센서2에서 감지된 시간 초기화
    sen1det = 0;                                      //센서1에서 감지확인 유무 초기화
    sen2det = 0;                                      //센서2에서 감지확인 유무 초기화
    lastcalcheck = 0;                                 //계산 유무 초기화
   
  }

  if(sen1det == 0 && sen2det == 0)  Serial.println("Not detect");  //센서1과 센서2에서 감지 안될경우 Not detect 출력
  
  unsigned int currentTime = millis();
  //현 시간 - 이전 시간 >= 1초일경우
  //==을 쓰지 않은 이유 : 아두이노를 구동후에 루프문을 돌리는데 걸리는 시간은 코드마다 다름
  //                   특히 여러번 진행됬을때 millis로 받은 시간이 딱 1000ms가 될 경우가 매우 희박함
  //                   한방에 1000ms를 맞추지 못 할 경우 조건문은 영원히 실행되지 않으므로 1000ms보다 같거나 클때라는 조건을 사용함
  //                   매우 작은 시간 오차를 감수하고 사용
  if((currentTime - oneSecPeriod) >= 1000){  
    oneSecPeriod = currentTime; //현 시간을 이전 시간으로 바꿈
    sec++;           //초 증가

    if(sec == 60){
      sec = 0;       //0초로 초기화
      minute++;      //분 증가

      if(minute == 60){
        minute = 0;  //0분으로 초기화
        hour++;      //시간 증가

        if(hour == 24){
          hour = 0; //0시로 초기화
          date++;   //일 증가 
          daycal++; //요일 변경
          
          if(date >= 29){
            if(month == 2){ //2월일 경우
              if((year % 4 == 0 && year % 100 != 0) || year % 400 == 0){   //윤년 계산, 4년과 400년마다, 100으로 나누어 떨어지지 않는 해일때             
                if(date == 30){ //윤년이 들어 30일이 됬을 경우
                  date = 1;     //1일로 초기화
                  month++;      //월 증가
                }
              }
              else{
                date = 1;       //1일로 초기화
                month++;        //월 증가
              }
            }
          }
          else if(date == 31){  //4, 6, 9, 11월일 경우
            if(month == 4 || month == 6 || month == 9 || month == 11){
              date = 1;         //1일로 초기화
              month++;          //월 증가
            }
          }
          else if(date == 32){  //나머지 1, 3, 5, 7, 8, 10, 12월일 경우
            date = 1;           //1일로 초기화
            month++;            //월 증가

            if(month == 13){  //월이 13이 넘을경우
              month = 1;      //1월로 초기화
              year++;         //년도 증가
            }
          }
        }
      }
    }

    //초 카운트하는 조건 안에 넣은 이유, 어차피 초 단위로 카운트해서 값을 초기화하기 때문
    if(lastcalcheck == 3 || lastcalcheck == 4){                     //하나의 센서에만 감지된 경우
      if(delcnt == 4){                                              //delcnt의 초기값은 1이므로 4 - 1 = 3초가 지날경우
        sen1time = 0;                                                //센서1에서 감지된 탈출시간 초기화
        sen2time = 0;                                               //센서2에서 감지된 탈출시간 초기화
        sen1det = 0;                                                //센서1에서의 감지 확인 초기화
        sen2det = 0;                                                //센서2에서의 감지 확인 초기화
        lastcalcheck = 0;                                           //시간 연산 유무 초기화
        delcnt = 0;                                                 //delcnt 초기화 (0으로  초기화 했지만 아래에 바로 +1을 해주므로 1부터 시작)
        Serial.println("clear");
      }
      delcnt++;                                                     //delcnt에 1을 더함
    }
    else  delcnt = 1;                                               //아닐경우 1로 초기화
  }

  if(cnt == 0){                                                     //사람이 없을 경우에
    lcd.noBacklight();                                              //백라이트 OFF
  }
  else  lcd.backlight();                                            //사람이 있을 경우에, 백라이트 ON
  
  lcd.setCursor(1, 0);

  lcd.print(year);
  lcd.print(".");

  if(month < 10)  lcd.print("0"); //10월 이전일때 앞에 0을 붙여 0X 형식으로 표시
  lcd.print(month);
  lcd.print(".");

  if(date < 10)  lcd.print("0");  //10일 이전일때 앞에 0을 붙여 0X형식으로 표시 
  lcd.print(date);
  lcd.print(".");

  lcd.print(day[daycal%7]);       //지난 일수 / 7 의 나머지를 이용하여 요일 표시

  lcd.setCursor(1, 1);

  if(hour > 11) ampmcnt = 1;      // 오후 12시 ~ 오후 23시 카운트
  else  ampmcnt = 0;              // 오전 0시 ~ 오전 11시 카운트
  lcd.print(ampm[ampmcnt]);       // AM, PM 표시
  

  if(ampmcnt == 0){               // 오전일 경우
    lcd.print(" ");
    if(hour < 10) lcd.print("0"); // 10시 이전일때 0을 붙여 0X 형식으로 표시
    lcd.print(hour);
  }
  else{                           // 오후일 경우
    if(hour == 12){               // 12시일 경우 12시 표시
      lcd.print(" ");     
      lcd.print(hour);
    }
    else{                         // 12시가 아닌 경우
      lcd.print(" ");
      if(hour < 22) lcd.print("0"); //10시 이전일때 0을 붙여 0X 형식으로 표시
      lcd.print(hour - 12);       // 시간 - 12시간으로 하여 표시
    }
  }
  lcd.print(":");

  if(minute < 10) lcd.print("0");   //10분 이전일때 0을 붙여 0X 형식으로 표시
  lcd.print(minute);
  lcd.print(":");

  if(sec < 10)  lcd.print("0");     //10초 이전일때 0을 붙여 0X 형식으로 표시
  lcd.print(sec);
  
  if(lastcalcheck == 0){
    analogWrite(3, 0);
    analogWrite(5, 255);
  }
  if(lastcalcheck != 0){
    analogWrite(3, 255);
    analogWrite(5, 0);
  }
  
  //한 센서에 짧은시간 중복하여 사람을 감지할 경우를 막기 위한 장치
  if(sen1delaycnt == LOW){                            //센서1 감지가 비활성화 됬을 경우
    sen1delaytime = millis();                         //센서1을 딜레이할 시간을 측정
    if(sen1delaytime - sen1ltdelaytime >= 1500){      //방금 측정한 값이 센서1에서 감지된 탈출 시점보다 1.5초 더 뒤일 경우에
      sen1delaycnt = HIGH;                            //센서1 감지 활성화
    }
  }  
  if(sen2delaycnt == LOW){                            //센서2 감지가 비활성화 됬을 경우
    sen2delaytime = millis();                         //센서2를 딜레이할 시간을 측정
    if(sen2delaytime - sen2ltdelaytime >= 1500){      //방금 측정한 값이 센서2에서 감지된 탈출 시점보다 1.5초 더 뒤일 경우에
      sen2delaycnt = HIGH;                            //센서2 감지 활성화
    }
  }
}
