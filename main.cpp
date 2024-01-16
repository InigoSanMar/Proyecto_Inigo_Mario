/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
 
#include "InterruptIn.h"
#include "mbed.h"
#include "Grove_LCD_RGB_Backlight.h"
#include "mbed_wait_api.h"

#define WAIT_TIME_MS 500 


// Definición de pines y componentes
InterruptIn botonReset(D3); // Botón de reset
AnalogIn Galga(A0);  // Pin analógico para la galga
DigitalIn botonTara(D2);   // Botón para tarar
DigitalOut ledRojo(D4);      // LED rojo
DigitalOut ledVerde(LED1);    // LED verde
DigitalOut Alarma(D6);    // Alarma
Grove_LCD_RGB_Backlight Pantalla(PB_9,PB_8); // Pantalla

// Estados de la máquina
enum Estado {
    REPOSO,
    CALIBRANDO,
    MIDIENDO,
    ALARMA
} estadoActual;

// Variables 
float pendiente;
float peso;
float voltaje0g;  // Voltaje calibrado 0g
float voltaje100g;  // Voltaje calibrado 100g
const float limitePeso = 120.0;  // Límite de peso en gramos
char mensajePeso[16];

// Variables para el temporizador de la alarma
Timer temporizadorAlarma;
const float tiempoLimiteAlarma = 3.0;  // Tiempo límite para desactivar la alarma en segundos

// Función para manejar el estado de parpadeo del LED rojo
void parpadearLED(Estado &estadoActual) {
    while (estadoActual == CALIBRANDO || estadoActual == ALARMA) { 
        ledRojo = !ledRojo;       // Alternar estado del LED
        wait_us(200000);
    }
}

// Función para manejar el estado de parpadeo de la alarma
void Alarmando(Estado &estadoActual) {
    while (estadoActual == ALARMA) { 
        Alarma = !Alarma;       // Alternar estado del LED
        wait_us(200000);       
    }
}

// float calcularPeso(float voltajeGalga) {
//    float pesoSalida;
    
//    while (true) {
//        pesoSalida = (Galga.read()*3.3-voltaje0g)/pendiente; // Cálculo del peso
//    }
//    return pesoSalida;
// }

void Resetear()
{
    Pantalla.clear();  
    Pantalla.print("Reseteando..."); 
    wait_us(1000000);
    thread_sleep_for(WAIT_TIME_MS);

    // Reset de variables
    temporizadorAlarma.reset(); 
    pendiente=0;
    voltaje0g = 0;
    voltaje100g = 0;    
    peso = 0;

    estadoActual = REPOSO; // Volver al estado de REPOSO
}

float calcularVoltajeMedio(float voltaje) { // Función que calcula el valor de voltaje medio medido para calibrar
    float valorMedido;
    int numMuestras = 1000;
    float voltajeAcumulado = 0.0;

    for (int i = 0; i < numMuestras; ++i) {
        valorMedido = voltaje;
        // Acumular los valores medidos
        voltajeAcumulado += valorMedido;
    }
    // Calcular el valor medio dividiendo la suma acumulada por el número de muestras
    float voltajeMedio = voltajeAcumulado / numMuestras;

    // Reset de las variables
    valorMedido = 0;
    voltajeAcumulado = 0.0;

    return voltajeMedio;
}


void enReposo(){

    ledRojo = 1;  // Enciende LED rojo
    ledVerde = 0;  // Apaga LED verde
    Alarma = 0;  // Apaga la alarma
    Pantalla.setRGB(0xff, 0xff, 0xff);
    Pantalla.clear();  
    Pantalla.locate(0,0); // Lo siguiente que se mande al LCD en primera fila, primera columna
    Pantalla.print("Quitar peso y "); // Escribe un texto fijo
    Pantalla.locate(0,1); // Lo siguiente lo escribe en la segunda fila, primera columna
    Pantalla.print("pulsar calibrar"); // Escribe un texto fijo
    thread_sleep_for(WAIT_TIME_MS);

    if (botonTara) {
        voltaje0g = calcularVoltajeMedio(Galga.read()*3.3);
        thread_sleep_for(WAIT_TIME_MS);
        Pantalla.clear();  
        Pantalla.locate(0,0); // Lo siguiente que se mande al LCD en primera fila, primera columna
        Pantalla.print("Coloque 100g"); // Escribe un texto fijo
        Pantalla.locate(0,1); // Lo siguiente lo escribe en la segunda fila, primera columna
        Pantalla.print("y pulse calibrar"); // Escribe un texto fijo
        thread_sleep_for(WAIT_TIME_MS);

        estadoActual = CALIBRANDO;
    }
}

void calibrando(){

    voltaje100g = calcularVoltajeMedio(Galga.read()*3.3);
    thread_sleep_for(WAIT_TIME_MS);

    parpadearLED(estadoActual);  // Parpadea el LED rojo
    ledVerde = 0;  // Apaga LED verde
    Alarma = 0;  // Apaga LED de alarma

    // Calculamos la pendeiente de la ecuación de la recta
    pendiente = (voltaje100g-voltaje0g)/100;
    
    Pantalla.clear();
    Pantalla.locate(0,0); 
    Pantalla.print("Calibración exitosa");
    Pantalla.locate(0,1); 
    Pantalla.print("pulse el boton de tara"); 
    thread_sleep_for(WAIT_TIME_MS);
                 
    if (botonTara) { // Cuando se pulsa el botón de tarar se pasa al modo de medición
        ledVerde = 1;  // Enciende el LED verde
        estadoActual = MIDIENDO;
    }
}


void midiendo(){

//   peso = calcularPeso(Galga.read()); 
    peso = (Galga.read()*3.3-voltaje0g)/pendiente; // Cálculo del peso
    Pantalla.clear();
    Pantalla.setRGB(0xff, 0xff, 0xff);  
    Pantalla.locate(0,0); 
    Pantalla.print("El peso es:"); 
    Pantalla.locate(0,1); 
  //  Pantalla.print("%.2f",peso); // Escribe un texto fijo
    thread_sleep_for(WAIT_TIME_MS); 
                
    if (peso > limitePeso) {
        estadoActual = ALARMA;
        temporizadorAlarma.reset();  // Reinicia el temporizador de alarma
        Pantalla.setRGB(0xff, 0xff, 0xff);  
        Pantalla.clear();
        Pantalla.locate(0,0);
        Pantalla.print("Exceso de peso"); // Escribe un texto fijo
        thread_sleep_for(WAIT_TIME_MS);
    }

}

void alarmando(){
    parpadearLED(estadoActual);  // Parpadea el LED rojo
    ledVerde = 0;  // Apaga LED verde
                
    if (temporizadorAlarma.read() > tiempoLimiteAlarma) { // Desactiva la alarma después de 3 segundos
        estadoActual = REPOSO;
        temporizadorAlarma.reset();  // Reinicia el temporizador de alarma
        Pantalla.clear();
        Pantalla.setRGB(0xff, 0xff, 0xff);  
        Pantalla.locate(0,0); 
        Pantalla.print("El peso es:");
        thread_sleep_for(WAIT_TIME_MS);
        } 
    else { // Activa y desactiva la alarma cada 0.25 segundos
            Alarmando(estadoActual);
         }
}   

// Función principal
int main() {
    
    botonReset.rise(Resetear); // InterruptIn para hacer un reset en cualquier momento 

    while (1) {
        switch (estadoActual) {
            case REPOSO: // Estado de reposo
                enReposo();
            break;

            case CALIBRANDO: // Estado de calibración
                calibrando();
                break;

            case MIDIENDO: // Estado de medición
                midiendo();
            break;

            case ALARMA: // Estado de alarma
                alarmando();
            break;
        }
        
    }
    
}
