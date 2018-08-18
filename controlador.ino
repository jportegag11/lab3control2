// Ejemplo_Timer
// Código original tomado de:
//      https://henryforceblog.wordpress.com/2015/05/02/blink-example-using-timer-on-a-tiva-launchpad/
// Modificado por Luis Alberto Rivera
// Julio, 2018

#include "wiring_private.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"

#define LED RED_LED

volatile uint8_t state = 0;
int analogRef = A11;
int analogPlanta = A9;
int val = 0;
int pin = 1;
float planta = 0;
float ref = 0;
const int pinCount = 8;
int digitalPins [] = {A0,A1,A2,A3,A4,A5,A6,A7};
int freq = 10000;
int flag = 0;
float a [] = {0,0.0709,0.929}; //tustin ajustado
//float a [] = {0,1,0}; 
//float b [] = {28.17,-49.49,21.74}; //tustin no ajustado
//float b [] = {14.17,-24.8,10.9}; // zpm no ajustado
float b [] = {27.17,-47.87,21.05}; // tustin ajustado
float e [] = {0,0,0};
float y [] = {0,0,0};
float eactual;
float yactual;

void setup(){
  // put your setup code here, to run once:
  pinMode(LED,OUTPUT);
  configureTimer1A(); // llamado a configuración del timer.
  for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    pinMode(digitalPins[thisPin], OUTPUT); 
  }
  Serial.begin(9600);
}

void loop(){ 
  if (flag == 1){
    ref = analogRead(analogRef);
    planta = analogRead(analogPlanta);
  
    ref = map(ref,0,4095,-15,15);
    planta = map(planta,0,4095,-15,15);
  
    eactual = ref - planta;
    yactual = 0; 
    for (int i = 2; i > 0; i = i - 1){
        e[i] = e[i-1];
        y[i] = y[i-1];
    }
    e[0] = eactual;
    y[0] = 0;
    for (int i = 0; i < 3; i = i + 1){
        
        yactual = yactual + b[i]*e[i] + a[i]*y[i];
    } 
    y[0] = yactual;
    if (yactual < -15){
      yactual = -15;
    }
    else if (yactual > 15){
      yactual = 15;
    }
    DAC( (int) yactual);
    //Serial.println(ref);
    
    flag = 0;

  }
}

// Función que configura el timer (1A en este ejemplo)
void configureTimer1A(){
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // Enable Timer 1 Clock
  ROM_IntMasterEnable(); // Enable Interrupts
  ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic
  
  // Configure Timer Frequency
  // Frequency is given by MasterClock / CustomValue
  // Examples: 80MHz / 80k = 1000 kHz ; 80MHz / 80M = 1 Hz
  ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, 80000000/freq); 

  // Al parecer, no hay función ROM_TimerIntRegister definida. Usar la de memoria FLASH
  // El prototipo de la función es:
  //    extern void TimerIntRegister(uint32_t ui32Base, uint32_t ui32Timer, void (*pfnHandler)(void));
  // Con el tercer argumento se especifica el handler de la interrupción (puntero a la función).
  // Usar esta función evita tener que hacer los cambios a los archivos internos de Energia,
  // sugeridos en la página de donde se tomó el código original.
  TimerIntRegister(TIMER1_BASE, TIMER_A, &Timer1AHandler);
  
  ROM_IntEnable(INT_TIMER1A);  // Enable Timer 1A Interrupt
  ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT); // Timer 1A Interrupt when Timeout
  ROM_TimerEnable(TIMER1_BASE, TIMER_A); // Start Timer 1A
}

// Handler (ISR) de la interrupción del Timer
void Timer1AHandler(void){
  //Required to launch next interrupt
  ROM_TimerIntClear(TIMER1_BASE, TIMER_A);
  flag = 1;
  state ^= 0x01; // Toggle led state (usa operación XOR por cada bit: state = state XOR 00000001b)
  digitalWrite(LED, state); // Blink
}
void DAC(int val){
    val = map(val,-15,15,0,255); // se mapea a 8 bits 
    pin = 1;
    for (int thisPin = 0; thisPin < pinCount; thisPin++) {
      if ((val & pin) == pin){ //se hace un AND con el valor de lectura de 8 bits y el bit correspondiente
        digitalWrite(digitalPins[thisPin], HIGH); //si es 1, se pone en HIGH el pin
      }
      else{
        digitalWrite(digitalPins[thisPin], LOW); //si es 0, se pone en LOW
      }
      pin = pin << 1;
    }
}


