// MOTOR SHIELD
// pinos que nao podem ser usadas
// 3 4 7 8 11 12 

// IMPORTANDO LIBS

#include <NanoSoftSensor.h>
#include <FiniteStateMachine.h>
#include <AFMotor.h>
#include <color.h>

// MAQUINA DE ESTADOS (COMPORTAMENTO)

State percebendo = State(percebendoEnter, percebendoUpdate, motorExit);
State equalizandoLDR = State(equalizandoLDREnter, equalizandoLDRUpdate, motorExit);
State procurandoLuz = State(procurandoLuzEnter, procurandoLuzUpdate, motorExit);
State estimulos = State(estimulosEnter, estimulosUpdate, motorExit);
State fuga = State(fugaEnter, fugaUpdate, motorExit);
State rezinha = State(rezinhaEnter, rezinhaUpdate, motorExit);
State andadinha = State(andadinhaEnter, andadinhaUpdate, motorExit);

FSM bot_anic = FSM(percebendo);

// CORES
// 1.0 - verde - planta
// 0.40 azul - parado
// 0.60 rosa - fuga
// 0.67 vermelho - nao pode interagir
// 0.75 amarelo - luz

// VARIAVEIS

const float cores[] = {
  1.0,0.40,0.60,0.67,0.90};

const int plant4[] = {50,350}; // range
const int plant5[] = {50,350}; // range

const int plant1pin = A4;
const int plant2pin = A5;
const int ldr1pin = A1;
const int ldr2pin = A3;

const int speed1 = 255;
const int speed2 = 255;

int plant1 = 0;
int plant2 = 0;

int ldr1 = 0;
int ldr2 = 0;

float HUE = 0;

boolean isRandom = 0;
boolean isRainbow = 0;

NanoSoftSensor ldrsuave1 = NanoSoftSensor(10);
NanoSoftSensor ldrsuave2 = NanoSoftSensor(10);

NanoSoftSensor plantronic1 = NanoSoftSensor(20);
NanoSoftSensor plantronic2 = NanoSoftSensor(20);

float p4 = 0;
float p5 = 0;

boolean cantEstimulatePlant = true;

int estimulos_count = 0;
int fuga_count = 0;
int luz_count = 0;
int rezinha_count = 0;
int andadinha_count = 0;

int difmin = 50;

AF_DCMotor Motor_Left(1, MOTOR12_64KHZ);
AF_DCMotor Motor_Right(2, MOTOR12_64KHZ);


int leds[] = {5,9,10}; 
Color cur_color = Color(1,1,1);
float hue = 0;

// SETUP

void setup() {

  Serial.begin(9600);

  Motor_Left.setSpeed(speed1);
  Motor_Right.setSpeed(speed1);

  pinMode(13, INPUT);
}

// LOOP

void loop() {

  ldr1 = ldrsuave1.update(1023 - analogRead(ldr1pin));
  ldr2 = ldrsuave2.update(1023 - analogRead(ldr2pin));

  // media plantronic

  int raw1 = analogRead(plant1pin);
  int raw2 = analogRead(plant2pin);

  plant1 = plantronic1.update(raw1);
  plant2 = plantronic2.update(raw2);

  p4 = float(plant1 - plant4[0]) / (plant4[1]-plant4[0]);
  p5 = float(plant2 - plant5[0]) / (plant5[1]-plant5[0]);


  if(cantEstimulatePlant == false && (p4 < .5 || p5 < .5)){
    cantEstimulatePlant = true;
    bot_anic.transitionTo(estimulos);
  }
  else if(cantEstimulatePlant == true && p4 > .5 && p5 > .5 ){
    cantEstimulatePlant = false;
  }

  if(digitalRead(13) == LOW){
    bot_anic.transitionTo(fuga);
  }

  // serial print

  Serial.print("ldr1 ");
  Serial.print(ldr1);
  Serial.print(" ldr2 ");
  Serial.print(ldr2);
  Serial.print(" esquerda4 ");
  Serial.print(plant1);
  Serial.print(" direita5 ");
  Serial.print(plant2);
  Serial.print(" p4 ");
  Serial.print(p4);
  Serial.print(" p5 ");
  Serial.print(p5);
  Serial.print(" ");
  Serial.print(digitalRead(13));
  Serial.println();

  bot_anic.update();

  //display_color(cur_color);
  rainbow();

}

// DESLIGA MOTOR EM TODAS AS SAIDAS DE ESTADOS

void motorExit(){
  isRandom = false;
  isRainbow = false;
  Motor_Left.run(RELEASE);
  Motor_Right.run(RELEASE);
  Serial.println("motor---Exit");
  delay(20);
}

// ESTADO PERCEPTIVO
void percebendoEnter() {
  //isRandom = true;
  isRainbow = true;
  Serial.println("percebendo---Enter");
}
void percebendoUpdate() {

  int dif = abs(ldr1-ldr2);

  if(dif > difmin){
    bot_anic.transitionTo(equalizandoLDR);
  }
  Serial.println("percebendo---Update");
  delay(20);
}

// ESTADO EQUALIZADOR DOS LDR
void equalizandoLDREnter() {
  isRainbow = true;
  if(ldr1<ldr2){
    Motor_Left.run(FORWARD);
    Motor_Right.run(BACKWARD);
  } 
  else {
    Motor_Left.run(BACKWARD);
    Motor_Right.run(FORWARD);
  }
  HUE = 0.75;
  Serial.println("equalizandoLDR---Enter");
}
void equalizandoLDRUpdate() {

  int dif = abs(ldr1-ldr2);

  if(dif < difmin){

    if(ldr1 > 700 && ldr2 > 700){ // aqui depende da intensidade da luz
      bot_anic.transitionTo(procurandoLuz);
    } 
    else {
      bot_anic.transitionTo(estimulos);
    }

  }
  Serial.println("equalizandoLDR---Update");
  delay(20);

}

// ESTADO PROCURANDO LUZ MAIS INTENSA
void procurandoLuzEnter() {
  isRainbow = true;
  luz_count = 0;
  Motor_Left.setSpeed(speed2);
  Motor_Right.setSpeed(speed2);
  Motor_Left.run(BACKWARD);
  Motor_Right.run(BACKWARD);
  Serial.println("procurandoLuz---Enter");
}
void procurandoLuzUpdate() {
  if(ldr1 > 900 && ldr2 > 900){
    bot_anic.transitionTo(rezinha);
  }
  if(luz_count > 50){
    bot_anic.transitionTo(equalizandoLDR);
  }

  luz_count++;
  int dif = abs(ldr1-ldr2);

  if(dif > difmin){
    bot_anic.transitionTo(equalizandoLDR);
  }

  Serial.println("procurandoLuz---Update");
  delay(20);
}

// ESTADO RECEBENDO ESTIMULOS
void estimulosEnter() {
  estimulos_count = 0;
  Motor_Left.setSpeed(speed2);
  Motor_Right.setSpeed(speed2);
  Motor_Left.run(FORWARD);
  Motor_Right.run(FORWARD);
  Serial.println("estimulos---Enter");
  HUE = 1;
}
void estimulosUpdate() {
  if(estimulos_count > 100){
    bot_anic.transitionTo(percebendo);
  }
  if(estimulos_count % 20){
    Motor_Left.run(RELEASE);
    Motor_Right.run(RELEASE);
    if(p4>p5){
      Motor_Left.setSpeed(speed2 -speed2/2);
      Motor_Right.setSpeed(speed2);
    } 
    else {
      Motor_Left.setSpeed(speed2);
      Motor_Right.setSpeed(speed2 -speed2/2);
    }
    Motor_Left.run(FORWARD);
    Motor_Right.run(FORWARD);    
  }
  estimulos_count++;
  Serial.print("estimulos---Update");
  Serial.println(estimulos_count);
  delay(20);
}

// ESTA FUGINDO
void fugaEnter() {
  fuga_count = 0;
  Motor_Left.setSpeed(speed2);
  Motor_Right.setSpeed(speed2);
  Motor_Left.run(BACKWARD);
  Motor_Right.run(BACKWARD);
  HUE = 0.60;
  Serial.println("fuga---Enter");
}
void fugaUpdate() {
  if(fuga_count > 30){
    bot_anic.transitionTo(equalizandoLDR);
  }
  fuga_count++;
  Serial.print("fuga---Update--");
  Serial.println(fuga_count);
  delay(20);
}

// REZINHA

void rezinhaEnter() {
  isRainbow = true;
  rezinha_count = 0;
  Motor_Left.setSpeed(speed2);
  Motor_Right.setSpeed(speed2);
  Motor_Left.run(BACKWARD);
  Motor_Right.run(BACKWARD);
  HUE = 0.60;
  Serial.println("rezinha---Enter");
}
void rezinhaUpdate() {
  if(rezinha_count > 15){
    bot_anic.transitionTo(percebendo);
  }
  rezinha_count++;
  Serial.print("rezinha---Update--");
  Serial.println(rezinha_count);
  delay(20);
}

// ANDADINHA

void andadinhaEnter() {
  isRainbow = true;
  andadinha_count = 0;
  Motor_Left.setSpeed(speed2);
  Motor_Right.setSpeed(speed2);
  Motor_Left.run(FORWARD);
  Motor_Right.run(FORWARD);
  HUE = 0.60;
  Serial.println("andadinha---Enter");
}
void andadinhaUpdate() {
  if(andadinha_count > 30){
    bot_anic.transitionTo(equalizandoLDR);
  }
  andadinha_count++;
  Serial.print("andadinha---Update--");
  Serial.println(andadinha_count);
  delay(20);
}

// COLOR

void rainbow(){
  if(isRandom){
    delay(1000);
    hue = cores[int(random(0,4))];
  } 
  else if(isRainbow){
    hue += 0.01;  
  } 
  else if(abs(HUE - hue) > 0.07){
    hue += 0.05;
  }
  if ( hue >=1 ) hue = 0;  
  float sat = 1.0;
  float val = 0.6;
  cur_color.convert_hcl_to_rgb(hue,sat,val);
  display_color(cur_color);
  /*
  Serial.print(HUE);
   Serial.print(' ');
   Serial.print(hue);
   Serial.println();//*/
  //delay(20);
}

void display_color(Color c){
  analogWrite(leds[0], c.red);
  analogWrite(leds[1], c.green);
  analogWrite(leds[2], c.blue);
  //delay(20);
}


boolean cycleCheck(unsigned long *lastMillis, unsigned int cycle)
{
  unsigned long currentMillis = millis();
  if(currentMillis - *lastMillis >= cycle)
  {
    *lastMillis = currentMillis;
    return true;
  }
  else
    return false;
}


/*
What pins are not used on the motor shield?
 
 All 6 analog input pins are available. They can also be used as digital pins (pins #14 thru 19)
 Digital pin 2, and 13 are not used.
 
 The following pins are in use only if the DC/Stepper noted is in use:
 Digital pin 11: DC Motor #1 / Stepper #1 (activation/speed control)
 Digital pin 3: DC Motor #2 / Stepper #1 (activation/speed control)
 Digital pin 5: DC Motor #3 / Stepper #2 (activation/speed control)
 Digital pin 6: DC Motor #4 / Stepper #2 (activation/speed control)
 
 The following pins are in use if any DC/steppers are used
 Digital pin 4, 7, 8 and 12 are used to drive the DC/Stepper motors via the 74HC595 serial-to-parallel latch
 
 The following pins are used only if that particular servo is in use:
 Digitals pin 9: Servo #1 control
 Digital pin 10: Servo #2 control
 
 */


