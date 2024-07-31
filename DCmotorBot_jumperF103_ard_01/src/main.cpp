#include <Wire.h>
#include <Arduino.h>
#define DEBUG

#define I2C_SLAVE_ADDRESS 0x55
// no MotorC port for this project
void pwm_rgb(int a0, int a1, int a2);
void set_rgb(int rgbarr[]);
int rgbarr[] = {0, 0, 0};
int rgbidx = 0;
HardwareSerial hs = HardwareSerial(PB7, PB6);
int a[6] = {0,0,0,0,0,0};
int a5 = 0;
int avg[6] = {0,0,0,0,0,0};

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


void receiveEvent(int byteCount) {
  if (Wire.available() == 6) {  // Expect 6 bytes: 3 for motors, 3 for RGB
    motorA_pwm = Wire.read();
    motorB_pwm = Wire.read();
    motorC_pwm = Wire.read();
    red_pwm = Wire.read();
    green_pwm = Wire.read();
    blue_pwm = Wire.read();
  }
}

int minS[6] = {10000, 10000, 10000,10000,10000,10000}; // sensor array min and max record
int maxS[6] = {0, 0, 0, 0, 0, 0};

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
  pinMode(PA0, INPUT_ANALOG); // ir a1
  pinMode(PA1, INPUT_ANALOG); //a2
  pinMode(PA2, INPUT_ANALOG);//a3
  pinMode(PA3, INPUT_ANALOG); // a4
  pinMode(PA4, INPUT_ANALOG); // a5
  pinMode(PA5, INPUT_ANALOG); // SEN1
  pinMode(PA6, INPUT_ANALOG); // SEN2
  pinMode(PA7, INPUT_ANALOG); // a6

  digitalWrite(PB2, 1); // on ir led
  pwm_rgb(0xFF, 0xFF, 0xFF);
  hs.begin(115200);
  // Initialize I2C as slave
  Wire.begin(I2C_SLAVE_ADDRESS);
  
  // Register the receive event
  Wire.onReceive(receiveEvent);
  delay(2000);


  // drive forward across a black line to calibrate
  for (int i = 0; i < 500; i++)
  {  apwm_ma(-1, 500);
      apwm_mb(-1, 500);
    // record min and max
    a[0] = analogRead(PA0);
    a[1] = analogRead(PA1);
    a[2] = analogRead(PA2);
    a[3] = analogRead(PA3);
    a[4] = analogRead(PA4);
    a[5] = analogRead(PA7);
    if (a[0] < minS[0]) minS[0] = a[0];
    else if (a[0] > maxS[0]) maxS[0] = a[0];
    if (a[1] < minS[1]) minS[1] = a[1];
    else if (a[1] > maxS[1]) maxS[1] = a[1];
    if (a[2] < minS[2]) minS[2] = a[2];
    else if (a[2] > maxS[2]) maxS[2] = a[2];
    if (a[3] < minS[3]) minS[3] = a[3];
    else if (a[3] > maxS[3]) maxS[3] = a[3];
    if (a[4] < minS[4]) minS[4] = a[4];
    else if (a[4] > maxS[4]) maxS[4] = a[4];
    if (a[5] < minS[5]) minS[5] = a[5];
    else if (a[5] > maxS[5]) maxS[5] = a[5];
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
  a[0] = analogRead(PA0);
  a[1] = analogRead(PA1);
  a[2] = analogRead(PA2);
  a[3] = analogRead(PA3);
  a[4] = analogRead(PA4);
  a[5] = analogRead(PA7);
  for(int i=0;i<6;i++)avg[i] = (minS[i] + maxS[i]) / 2;
  
  // a5 = analogRead(PA5);
  // hs.printf("%d\t%d\t%d\r\n",a0,a1,a2);

  if (in_range(maxS[0], minS[0], a[0]) &&
      in_range(maxS[1], minS[1], a[1]) &&
      in_range(maxS[2], minS[2], a[2]) &&
      in_range(maxS[3], minS[3], a[3]) &&
      in_range(maxS[4], minS[4], a[4]) &&
      in_range(maxS[5], minS[5], a[5]))
  {
    pwm_rgb(0xFF, 0xFF, 0xFF);
    // white is high, maxS, black is minS low
    // 0 ~ 1000

    #ifdef DEBUG
      return;
    #endif

    if(similar(a[0], a[1], a[2])){
      apwm_ma(-1, 1000);
      apwm_mb(-1, 1000);
      return;
    }

    if(a[0] < avg[0]){
      apwm_ma(-1, 500);
      apwm_mb(1, 500);
      return;
    }

    if(a[2] < avg[2]){
      apwm_ma(1, 500);
      apwm_mb(-1, 500);
      return;
    }

    if(a[1] < avg[1]){
      apwm_ma(-1, 600);
      apwm_mb(-1, 300);
    }else{
      apwm_ma(-1, 300);
      apwm_mb(-1, 600);
    }
    
  } else {
    
  }

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
