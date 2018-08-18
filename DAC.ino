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
int analogPin = A11;
int val = 0;
int pin = 1;
const int pinCount = 8;
int digitalPins [] = {A0,A1,A2,A3,A4,A5,A6,A7};
int freq = 10000;

void setup(){
  // put your setup code here, to run once:
  pinMode(LED,OUTPUT);
  configureTimer1A(); // llamado a configuración del timer.
  for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    pinMode(digitalPins[thisPin], OUTPUT); 
  }
}

void loop(){
  // El loop principal no hace nada. Todo se realiza en el handler de la interrupción, cuando
  // se vence el timer.
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
  
  state ^= 0x01; // Toggle led state (usa operación XOR por cada bit: state = state XOR 00000001b)
  digitalWrite(LED, state); // Blink
  //val = analogRead(analogPin); //se obtiene el valor de entrada
  val = 0;
  DAC(val);
}
  void DAC(int val){
    val = map(val,0,4095,0,255); // se mapea a 8 bits 
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


