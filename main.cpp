#include "DigitalIn.h"
#include "Grove_LCD_RGB_Backlight.h"
#include "mbed.h"
#include "mbed_wait_api.h"

#define WAIT_TIME_MS 5

// Definición de pines y componentes
DigitalIn botonReset(D3); // Botón de reset
AnalogIn Galga(A0);  // Pin analógico para la galga
DigitalIn boton(D2);   // Botón para tarar
DigitalOut ledRojo(D4);      // LED rojo
DigitalOut ledVerde(LED1);    // LED verde
DigitalOut Alarma(D6);    // Alarma
Grove_LCD_RGB_Backlight Pantalla(PB_9,PB_8); // Pantalla

Timer temporizador; //

enum estados {
  Reposo,
  Calibracion0g,
  Calibracion100g,
  Midiendo,
  Tarando,
  Alarmando
} estado;

float peso;
float voltajeMedio0g;
float voltajeMedio100g;
float pendiente;
float tara;
char mensajePeso[100];

void CalcularPeso() { 
            pendiente = (voltajeMedio100g-voltajeMedio0g)/100;
            peso = ((Galga*3.3-voltajeMedio0g)/pendiente)-tara;
    
     }

// Función para manejar el estado de parpadeo del LED rojo
void parpadearLED() {
    while (estado == Calibracion0g || estado == Calibracion100g || estado == Alarmando) { 
        ledRojo = !ledRojo;       // Alternar estado del LED
        wait_us(200000);
    }
}

// Función para manejar la activación de la alarma
void activarAlarma() {
    while (estado == Alarmando) { 
        Alarma = !Alarma;       // Alternar estado del LED
        wait_us(200000);       
    }
}

float calcularMediaVoltaje0g(float Voltaje0g){
    int numMuestras = 200;
    float voltajeAcumulado = 0.0;

    for (int i = 0; i < numMuestras; ++i) {
        // Acumular los valores medidos
        voltajeAcumulado += Voltaje0g;
        wait_us(10000);
    }
    // Calcular el valor medio dividiendo la suma acumulada por el número de muestras
    voltajeMedio0g = voltajeAcumulado / numMuestras;

    return voltajeMedio0g;

}

float calcularMediaVoltaje100g(float Voltaje100g){
    int numMuestras = 200;
    float voltajeAcumulado = 0.0;

    for (int i = 0; i < numMuestras; ++i) {
        // Acumular los valores medidos
        voltajeAcumulado += Voltaje100g;
        wait_us(20000);
    }
    // Calcular el valor medio dividiendo la suma acumulada por el número de muestras
    voltajeMedio100g = voltajeAcumulado / numMuestras;

    return voltajeMedio100g;

}


void estadoReposo() {

    ledRojo = 1;  // Enciende LED rojo
    ledVerde = 0;  // Apaga LED verde
    Alarma = 0;  // Apaga la alarma
    Pantalla.clear();  
    Pantalla.locate(0,0); // Lo siguiente que se mande al LCD en primera fila, primera columna
    Pantalla.print("Quitar el peso "); // Escribe un texto fijo
    Pantalla.locate(0,1); // Lo siguiente lo escribe en la segunda fila, primera columna
    Pantalla.print("y pulsar boton"); // Escribe un texto fijo
    thread_sleep_for(WAIT_TIME_MS);
    
  if (boton == 1) {
        voltajeMedio0g = calcularMediaVoltaje0g(Galga*3.3);
        estado = Calibracion0g;
    
  }

}



void estadoCalibracion0g() {

    Pantalla.clear();  
    Pantalla.locate(0,0);
    Pantalla.print("Coloque 100g"); 
    Pantalla.locate(0,1); 
    Pantalla.print("y pulsar boton"); 
    thread_sleep_for(WAIT_TIME_MS);

    if (estado == Calibracion0g) { 
        ledRojo = !ledRojo;       // Alternar estado del LED
        wait_us(2000);
    }

  if (boton == 1) {  
        voltajeMedio100g = calcularMediaVoltaje100g(Galga*3.3);  
        estado = Calibracion100g;
  }

    if (botonReset == 1){

        Pantalla.setRGB(0xff, 0xff, 0xff);
        Pantalla.clear();  
        Pantalla.locate(0,0);
        Pantalla.print("Reseteando");
        thread_sleep_for(WAIT_TIME_MS); 
        wait_us(2000000);

        estado = Reposo;

  }
}


void estadoCalibracion100g() {

    Pantalla.clear();  
    Pantalla.locate(0,0);
    Pantalla.print("Retirar peso"); 
    Pantalla.locate(0,1); 
    Pantalla.print("y pulsar boton"); 
    thread_sleep_for(WAIT_TIME_MS);

    if (estado == Calibracion100g) { 
        ledRojo = !ledRojo;       // Alternar estado del LED
    }

  if (boton == 1) {
        ledRojo = 0; 
        ledVerde = 1;  
        Alarma = 0;         
        
        estado = Midiendo;

  }

    if (botonReset == 1){

        Pantalla.setRGB(0xff, 0xff, 0xff);
        Pantalla.clear();  
        Pantalla.locate(0,0);
        Pantalla.print("Reseteando");
        thread_sleep_for(WAIT_TIME_MS); 
        wait_us(2000000);

        estado = Reposo;

  }
}

void estadoMidiendo() {

        Pantalla.setRGB(0x00, 0x80, 0x00);
        Pantalla.clear();  
        Pantalla.locate(0,0);
        Pantalla.print("El peso es:"); 
        sprintf(mensajePeso, "%.2f g", peso);
        Pantalla.locate(0,1); 
        Pantalla.print(mensajePeso);
        wait_us(1500000);
        thread_sleep_for(WAIT_TIME_MS); 
  
  if (peso >= 130) {
        ledVerde = 0;  
        estado = Alarmando;
  }

  if (boton == 1) {
      tara = peso;
      estado = Tarando;

  }

  if (botonReset == 1){

        Pantalla.setRGB(0xff, 0xff, 0xff);
        Pantalla.clear();  
        Pantalla.locate(0,0);
        Pantalla.print("Reseteando");
        thread_sleep_for(WAIT_TIME_MS); 
        wait_us(2000000);

        estado = Reposo;

  }
}

void estadoTarando(){

        Pantalla.setRGB(0xff, 0xff, 0xff);
        Pantalla.clear();  
        Pantalla.locate(0,0);
        Pantalla.print("Tarando");
        thread_sleep_for(WAIT_TIME_MS); 
        wait_us(2000000);

        peso = peso - tara;
        estado = Midiendo;
        if (botonReset == 1){

            Pantalla.setRGB(0xff, 0xff, 0xff);
            Pantalla.clear();  
            Pantalla.locate(0,0);
            Pantalla.print("Reseteando");
            thread_sleep_for(WAIT_TIME_MS); 
            wait_us(2000000);

            estado = Reposo;
  }
}

void estadoAlarmando() {

       if (estado == Alarmando) { 
        ledRojo = !ledRojo;       // Alternar estado del LED
        Alarma = !Alarma;
        wait_us(2000);
    }
 
    temporizador.reset();
    temporizador.start();

    Pantalla.setRGB(0xff, 0x00, 0x00);
    Pantalla.clear();  
    Pantalla.locate(0,0);
    Pantalla.print("ALARMA");
    Pantalla.locate(0,1); 
    Pantalla.print("Exceso de peso");
    thread_sleep_for(WAIT_TIME_MS);

  if (peso < 120) {
        ledRojo = 0; 
        ledVerde = 1;  
        Alarma = 0;  
        estado = Midiendo; 
  }

    if (botonReset == 1){

        Pantalla.setRGB(0xff, 0xff, 0xff);
        Pantalla.clear();  
        Pantalla.locate(0,0);
        Pantalla.print("Reseteando");
        thread_sleep_for(WAIT_TIME_MS); 
        wait_us(2000000);

        estado = Reposo;

  }
  
}


int main() {
  tara = 0;
  temporizador.reset();
  Pantalla.setRGB(0xff, 0xff, 0xff);
  estado = Reposo;

  while (true) {
    CalcularPeso();
    thread_sleep_for(WAIT_TIME_MS);

    switch (estado) {
    case Reposo:
      estadoReposo();
      break;
    case Calibracion0g:
      estadoCalibracion0g();
      break;
    case Calibracion100g:
      estadoCalibracion100g();
      break;
    case Midiendo:
      estadoMidiendo();
      break;
    case Tarando:
        estadoTarando();
        break;
    case Alarmando:
      estadoAlarmando();
      break;
    }

    thread_sleep_for(WAIT_TIME_MS);
  }
}
