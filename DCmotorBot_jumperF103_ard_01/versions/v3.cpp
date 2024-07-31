#include <Arduino.h>
// no MotorC port for this project
void pwm_rgb(int a0, int a1, int a2);
void set_rgb(int rgbarr[]);
int rgbarr[] = {0, 0, 0};
int rgbidx = 0;
HardwareSerial hs = HardwareSerial(PB7, PB6);
int a0 = 0;
int a1 = 0;
int a2 = 0;
int a5 = 0;

int avg0 = 0;
int avg1 = 0;
int avg2 = 0;

// long PWM_MANUAL =  12000;  //range of the pwm is from 0 to 12000 microseconds.
long PWM_MANUAL = 1000;
void move_ma(int dir)
{
  switch (dir)
  {
  case 1:
    digitalWrite(PA9, 0);
    digitalWrite(PA8, 1);
    break;
  case -1:
    digitalWrite(PA8, 0);
    digitalWrite(PA9, 1);
    break;
  case 0:
    digitalWrite(PA8, 0);
    digitalWrite(PA9, 0);
  case 3:
  default:
    digitalWrite(PA8, 1);
    digitalWrite(PA9, 1);
    break;
  }
}

void move_mb(int dir)
{
  switch (dir)
  {
  case 1:
    digitalWrite(PB3, 0);
    digitalWrite(PA15, 1);
    break;
  case -1:
    digitalWrite(PA15, 0);
    digitalWrite(PB3, 1);
    break;
  case 0:
    digitalWrite(PA15, 0);
    digitalWrite(PB3, 0);
  case 3:
  default:
    digitalWrite(PA15, 1);
    digitalWrite(PB3, 1);
    break;
  }
}

void apwm_ma(int dir, int pwr)
{
  if (pwr > PWM_MANUAL)
    return;
  long at = micros();
  while ((micros() - at) < pwr)
    move_ma(dir);
  move_ma(0);
  delayMicroseconds(PWM_MANUAL - pwr);
}
void apwm_mb(int dir, int pwr)
{
  if (pwr > PWM_MANUAL)
    return;
  long at = micros();
  while ((micros() - at) < pwr)
    move_mb(dir);
  move_mb(0);
  delayMicroseconds(PWM_MANUAL - pwr);
}

int minS[3] = {10000, 10000, 10000}; // sensor array min and max record
int maxS[3] = {0, 0, 0};

void setup()
{

  pinMode(PA8, OUTPUT); // MA
  pinMode(PA9, OUTPUT);
  pinMode(PA15, OUTPUT); // MB
  pinMode(PB3, OUTPUT);
  move_ma(0);
  move_mb(0);
  pinMode(PB13, OUTPUT_OPEN_DRAIN);
  pinMode(PB14, OUTPUT_OPEN_DRAIN);
  pinMode(PB15, OUTPUT_OPEN_DRAIN);
  pinMode(PB2, OUTPUT);       // turn on IR leds
  pinMode(PA0, INPUT_ANALOG); // ir x3
  pinMode(PA1, INPUT_ANALOG);
  pinMode(PA2, INPUT_ANALOG);
  pinMode(PA5, INPUT_ANALOG); // SEN1

  digitalWrite(PB2, 1); // on ir led
  pwm_rgb(0xFF, 0xFF, 0xFF);
  hs.begin(115200);
  delay(2000);


  // drive forward across a black line to calibrate
  for (int i = 0; i < 500; i++)
  {  apwm_ma(-1, 500);
      apwm_mb(-1, 500);
    // record min and max
    a0 = analogRead(PA0);
    a1 = analogRead(PA1);
    a2 = analogRead(PA2);
    if (a0 < minS[0])
      minS[0] = a0;
    else if (a0 > maxS[0])
      maxS[0] = a0;
    if (a1 < minS[1])
      minS[1] = a1;
    else if (a1 > maxS[1])
      maxS[1] = a1;
    if (a2 < minS[2])
      minS[2] = a2;
    else if (a2 > maxS[2])
      maxS[2] = a2;
  }
  move_ma(0);
  move_mb(0);
  delay(100);
  pwm_rgb(0x00, 0x00, 0x00);
  delay(100);
  pwm_rgb(0xFF, 0xFF, 0xFF);
  delay(100);
  pwm_rgb(0x00, 0x00, 0x00);
  delay(100);
  pwm_rgb(0xFF, 0xFF, 0xFF);
}
#define RNG_TOLERANCE 20
bool in_range(int high, int low, int now)
{
  if (now > (high + RNG_TOLERANCE))
    return false;
  else if (now < (low - RNG_TOLERANCE))
    return false;
  else
    return true;
}

bool similar(int n0, int n1, int n2){
  return (abs(n0-n1)<RNG_TOLERANCE && abs(n1-n2)<RNG_TOLERANCE);
}

void loop()
{

  // move_ma(1);
  // move_mb(1);
  // delay(2000);
  // int step = PWM_MANUAL / 50;
  // for(int i=0;i<PWM_MANUAL;i+=step){
  //   apwm_ma(1,i);
  //   apwm_mb(1,i);
  // }
  // for(int i=0;i<PWM_MANUAL;i+=step){
  //   apwm_ma(1,PWM_MANUAL);
  //   apwm_mb(1,PWM_MANUAL);
  // }

  // rgbidx++;
  // if(rgbidx>100)rgbidx=0;
  // rgbarr[0]=1;
  // rgbarr[1]=1;
  // rgbarr[2]=1;
  // if(rgbidx%3==1)    rgbarr[0] = 0;
  // if((rgbidx+1)%4==1)rgbarr[1] = 0;
  // if((rgbidx+2)%5==1)rgbarr[2] = 0;
  // set_rgb(rgbarr);
  // delay(30);

  a0 = analogRead(PA0);
  a1 = analogRead(PA1);
  a2 = analogRead(PA2);
  avg0 = (minS[0] + maxS[0]) / 2;
  avg1 = (minS[1] + maxS[1]) / 2;
  avg2 = (minS[2] + maxS[2]) / 2;
  // a5 = analogRead(PA5);
  // hs.printf("%d\t%d\t%d\r\n",a0,a1,a2);

  // int amp = PWM_MANUAL / 20;
  // use 3 ir sensors to tell direction
  // if(b0<b1 || b0<b2){ //left MB > MA
  // apwm_ma(-1,a2 * amp);
  // apwm_mb(-1,a0 * amp);
  // }
  // if(b1<b0 || b1<b2){// straight
  //   apwm_ma(-1,500);
  //   apwm_mb(-1,500);
  // }
  // if(b2<b1 || b2<b0){ //left MB > MA
  //   apwm_ma(-1,1000);
  //   apwm_mb(-1,100);
  // }

  if (in_range(maxS[0], minS[0], a0) &&
      in_range(maxS[1], minS[1], a1) &&
      in_range(maxS[2], minS[2], a2))
  {
    pwm_rgb(0xFF, 0xFF, 0xFF);
    // white is high, maxS, black is minS low
    // 0 ~ 1000

    if(similar(a0, a1, a2)){
      apwm_ma(-1, 1000);
      apwm_mb(-1, 1000);
      return;
    }

    if(a0 < avg0){
      apwm_ma(-1, 500);
      apwm_mb(1, 500);
      return;
    }

    if(a2 < avg2){
      apwm_ma(1, 500);
      apwm_mb(-1, 500);
      return;
    }

    if(a1 < avg1){
      apwm_ma(-1, 600);
      apwm_mb(-1, 300);
    }else{
      apwm_ma(-1, 300);
      apwm_mb(-1, 600);
    }
    
    // if (a0 < avg0 && a0 > minS[0]) apwm_ma(-1, 600);
    // if (a2 < avg2 && a2 > minS[2]) apwm_mb(-1, 600);
        // if (a1 > avg1)
    //   {
    //     apwm_ma(-1, 1000);
    //     apwm_mb(-1, 100);
    //   }
    //   else
    //   {
    //     apwm_ma(-1, 100);
    //     apwm_mb(-1, 1000);
    //   }
  }
  else
  {
    // pwm_rgb(0xFF - a0 / 10, 0xFF - a1 / 10, 0xFF - a2 / 10);

    // maxS[] is the white color and minS is the black color, anything lower than minS is too far away.
    // use average of maxS and minS of [1] as middle point
  }

  // hs.printf("%d\t%d\t%d\t%d\r\n", a0, a1, a2, a5);
  // pwm_rgb(a5, 0xff, 0xff);

  // int prange = 1024;
  // for (int i = 0; i < prange; i++)
  // {
  //    move_ma(1);
  //    move_mb(1);
  //    delayMicroseconds(prange-i);
  //    move_ma(0);
  //    move_mb(0);
  //    delayMicroseconds(i);
  // }

  // for (int i = 255; i > 0; i--)
  // {
  //    move_ma(1);
  //    move_mb(1);
  //   delay(5);
  // }
  // for (int i = 0; i < 255; i++)
  // {
  //   // digitalWrite(PA15,0);
  //   // analogWrite(PB3, i);
  //   digitalWrite(PA_8,0);
  //   analogWrite(PA_9, i);
  //   delay(10);
  // }
  // for (int i = 0; i < 254; i++){
  //     pwm_rgb(0xff,i,0xff);
  //     delay(1);
  // }

  // This worked with direction
  //  move_ma(1);
  //  move_mb(-1);
  //  delay(1000);
  //  move_ma(0);
  //  move_mb(0);
  //  delay(1000);
  //  move_ma(-1);
  //  move_mb(1);
  //  delay(1000);
  //  move_mb(0);
  //  move_ma(0);
  //  delay(1000);
}

void set_rgb(int rgbarr[])
{
  digitalWrite(PB13, rgbarr[0]);
  digitalWrite(PB14, rgbarr[1]);
  digitalWrite(PB15, rgbarr[2]);
}

void pwm_rgb(int x, int y, int z)
{
  analogWrite(PB13, x);
  analogWrite(PB14, y);
  analogWrite(PB15, z);
}
