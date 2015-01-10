// MOTOR SHIELD
// pinos que nao podem ser usadas
// 3 4 7 8 11 12 

// IMPORTANDO LIBS

#include <FiniteStateMachine.h>
#include <AFMotor.h>
#include <color.h>

// MAQUINA DE ESTADOS (COMPORTAMENTO)

State percebendo = State(percebendoEnter, percebendoUpdate, motorExit);
State equalizandoLDR = State(equalizandoLDREnter, equalizandoLDRUpdate, motorExit);
State procurandoLuz = State(procurandoLuzEnter, procurandoLuzUpdate, motorExit);
State estimulos = State(estimulosEnter, estimulosUpdate, motorExit);
State fuga = State(fugaEnter, fugaUpdate, motorExit);

FSM bot_anic = FSM(percebendo);

// VARIAVEIS

#define NUMREADINGS 10

const int ldr1pin = A1;
const int ldr2pin = A3;

const int plant1pin = A4;
const int plant2pin = A5;

const int speed1 = 255;
const int speed2 = 255;

int ldr1 = 0;
int ldr2 = 0;

const int minPlanta = 400;
const int maxPlanta = 420;

int index = 0;
int plant1 = 0;
int plant2 = 0;
int plant1total = 0;
int plant2total = 0;
int plant1arr[NUMREADINGS];
int plant2arr[NUMREADINGS];

boolean plant = true;

int difmin = 100;// resolucao do LDR --- tentar com 50 ou menor. Qt menor mais sensivel

AF_DCMotor Motor_Left(2, MOTOR12_64KHZ);
AF_DCMotor Motor_Right(4, MOTOR12_64KHZ);


int leds[] = {5,9,10}; 
Color cur_color = Color(1,1,1);
float hue = 0;

// SETUP

void setup() {
  
  Serial.begin(9600);

  Motor_Left.setSpeed(speed1);
  Motor_Right.setSpeed(speed1);
  
   for (int i = 0; i < NUMREADINGS; i++){
     plant1arr[i] = 0;
     plant2arr[i] = 0;
   }
   
   for(int j = 0 ; j < 3; j++ ){
    //pinMode(leds[j], OUTPUT); 
   }

}

// LOOP

void loop() {

  ldr1 = 1023 - analogRead(ldr1pin);
  ldr2 = 1023 - analogRead(ldr2pin);
  
  // media plantronic
  
  plant1total -= plant1arr[index];
  plant2total -= plant2arr[index];

  int teste1 = plant1arr[index] = analogRead(plant1pin);
  int teste2 = plant2arr[index] = analogRead(plant2pin);
  
  plant1total += plant1arr[index];
  plant2total += plant2arr[index];
  
  index = (index + 1) % NUMREADINGS;

  plant1 = plant1total / NUMREADINGS;
  plant2 = plant2total / NUMREADINGS;
  
  if(!plant && plant2 < maxPlanta){
    Serial.println('PLANTTTTT');
    plant = true;
    bot_anic.transitionTo(estimulos);
  }
  else if(plant && plant2 > maxPlanta){
    plant = false;
  }
  
  // serial print
  
  Serial.print(ldr1);
  Serial.print("\t");
  Serial.print(ldr2);
  Serial.print("\t");
  Serial.print(plant1);
  Serial.print("\t");
  Serial.print(plant2);
  Serial.print("\t");
  Serial.print(teste1);
  Serial.print("\t");
  Serial.print(teste2);
  Serial.println();
  
  bot_anic.update();
  
  //display_color(cur_color);
  rainbow();
  
}

// DESLIGA MOTOR EM TODAS AS SAIDAS DE ESTADOS

void motorExit(){
  Motor_Left.run(RELEASE);
  Motor_Right.run(RELEASE);
  Serial.println("motor---Exit");
  delay(100);
}

// ESTADO PERCEPTIVO
void percebendoEnter() {
  Serial.println("percebendo---Enter");
}
void percebendoUpdate() {
  
  int dif = abs(ldr1-ldr2);
  
  if(dif > difmin){
    bot_anic.transitionTo(equalizandoLDR);
  }
  Serial.println("percebendo---Update");
  delay(100);
}

// ESTADO EQUALIZADOR DOS LDR
void equalizandoLDREnter() {
  if(ldr1>ldr2){
    Motor_Left.run(FORWARD);
    Motor_Right.run(BACKWARD);
  } else {
    Motor_Left.run(BACKWARD);
    Motor_Right.run(FORWARD);
  }
  Serial.println("equalizandoLDR---Enter");
}
void equalizandoLDRUpdate() {
  
  int dif = abs(ldr1-ldr2);
  
  if(dif < difmin){
    
    if(ldr1 > 500 && ldr2 > 500){ // aqui depende da intensidade da luz
      bot_anic.transitionTo(procurandoLuz);
    } else {
      bot_anic.transitionTo(percebendo);
    }
    
  }
  Serial.println("equalizandoLDR---Update");
  delay(100);
  
}

// ESTADO PROCURANDO LUZ MAIS INTENSA
void procurandoLuzEnter() {
  Motor_Left.setSpeed(speed2);
  Motor_Right.setSpeed(speed2);
  Motor_Left.run(BACKWARD);
  Motor_Right.run(BACKWARD);
  Serial.println("procurandoLuz---Enter");
}
void procurandoLuzUpdate() {
  if(ldr1 > 700 && ldr2 > 700){
    bot_anic.transitionTo(percebendo);
  }
  
  int dif = abs(ldr1-ldr2);
  
  if(dif > difmin){
    bot_anic.transitionTo(equalizandoLDR);
  }
  
  Serial.println("procurandoLuz---Update");
  delay(100);
}

// ESTADO RECEBENDO ESTIMULOS
void estimulosEnter() {
  Motor_Left.setSpeed(speed2);
  Motor_Right.setSpeed(speed2);
  Motor_Left.run(FORWARD);
  Motor_Right.run(FORWARD);
  Serial.println("estimulos---Enter");
}
void estimulosUpdate() {
  
  if(digitalRead(13) == LOW){
    bot_anic.transitionTo(fuga);
  }
  else if(plant2 < minPlanta || plant2 > maxPlanta){
    bot_anic.transitionTo(percebendo);
  }
  
  Serial.println("estimulos---Update");
  delay(100);
}


// COLOR

void rainbow(){
  hue += 0.01;
  if ( hue >=1 ) hue = 0;
  float sat = 1.0;
  float val = 0.4;
  cur_color.convert_hcl_to_rgb(hue,sat,val);
  display_color(cur_color);
}

void display_color(Color c){
  analogWrite(leds[0], c.red);
  analogWrite(leds[1], c.green);
  analogWrite(leds[2], c.blue);
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

