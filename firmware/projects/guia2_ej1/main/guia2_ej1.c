/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 * 
 * 1) Mostrar distancia medida utilizando los leds de la siguiente manera:
 * Si la distancia es menor a 10 cm, apagar todos los LEDs.
 * Si la distancia está entre 10 y 20 cm, encender el LED_1.
 * Si la distancia está entre 20 y 30 cm, encender el LED_2 y LED_1.
 * Si la distancia es mayor a 30 cm, encender el LED_3, LED_2 y LED_1.
 * 2) Mostrar el valor de distancia en cm utilizando el display LCD.
 * 3) Usar TEC1 para activar y detener la medición.
 * 4) Usar TEC2 para mantener el resultado (“HOLD”).
 * 5) Refresco de medición: 1 s

 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	GPIO_3	 	| 	ECHO		|
 * | 	GPIO_2		|	TRIGGER		|
 * |	+5V			|	+5V			|
 * |	GND			|	GND			|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 5/09/2024 | Document creation		                         |
 *
 * @author Lonardi, Paula (paula.lonardi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include "hc_sr04.h"
#include "lcditse0803.h"
#include <led.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define PERIODO_MEDICION 1000
#define PERIODO_MUESTREO 500
#define PERIODO_LECTURA_TECLAS 200


/*==================[internal data definition]===============================*/
uint16_t distancia;
bool encendido;
bool hold;
uint8_t teclas;


/*==================[external functions definition]==========================*/
/**
 * @brief Actualiza los leds, encendiendo o apagando según corresponda
 *
 * Esta función se encarga de actualizar el estado de los leds
 * - Si la distancia es menor a 10, se apagan todos
 * - Si la distancia está entre 10 y 20 se prende el led 1
 * - Si la distancia está entre 20 y 30 se prende el led 1 y 2
 * - Si la distancia es mayor a 30, se prenden todos
 * 
 * 
 * 
 * @param void
 *
 * @return void
 */
void actualizar_led(){
	if (distancia < 10)
    	{
		LedOff(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
		}
		else if(distancia > 10 && distancia < 20)
		{
		LedOn(LED_1);
		LedOff(LED_2);
		LedOff(LED_3);
		}
		else if(distancia > 20 && distancia < 30)
		{
		LedOn(LED_1);
		LedOn(LED_2);
		LedOff(LED_3);
		}
		else if(distancia > 30)
		{
		LedOn(LED_1);
		LedOn(LED_2);
		LedOn(LED_3);
		}
				
}


/*==================[internal functions declaration]=========================*/
/**
 * @brief TAREA que mide la distancia utilizando el sensor ultrasónico HC-SR04.
 *
 * Tarea (se ejecuta en un bucle infinito). Si está "encendido" (la medición), la tarea mide y guarda la distancia utilizando la función "HcSr04ReadDistanceInCentimeters()".
 * La tarea entra en espera utilizando "ulTaskNotifyTake()" hasta que recibe una notificación para volver a ejecutar el proceso de medición.
 *
 * @param[in] pvParameter Puntero a los parámetros pasados a la tarea.
 *
 * @return void
 */
void tarea_medir(void *pvParameter){
	while (1)
	{ 
		if (encendido)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		vTaskDelay(PERIODO_MEDICION / portTICK_PERIOD_MS);
	}
	
}


/**
 * @brief TAREA encargada de mostrar la mediciónmdiante LEDs y un display LCD.
 *
 * Tarea que se ejecuta en un bucle infinito. Si el sistema está encendido, actualiza el estado de los LEDs, 
 * muestra la distancia en la terminal serie y, si no está en modo "hold" (mostrando una medición fija), 
 * también muestra la distancia en el display LCD. Si el sistema está apagado, apaga los LEDs y el LCD.
 *
 * @param[in] pvParameter Puntero a los parámetros pasados a la tarea.
 *
 * @return void
*/
void tarea_mostrar(void *pvParameter){
	while (1)
	{
		if (encendido)
		{
			actualizar_led();
			
			if (hold == false)
			{
				LcdItsE0803Write(distancia);
			}

		}
	vTaskDelay(PERIODO_MUESTREO / portTICK_PERIOD_MS);

	}
	
}


/**
 * @brief TAREA encargada de leer el estado de las teclas.
 *
 * Se verifica constantemente el estado de los botones conectados.
 * Si se presiona SWITCH_1, activa la medición; si se presiona SWITCH_2, desactiva el modo "hold".
 * Si no se presiona ningún botón, desactiva la medición y activa el modo "hold".
 *
 * @param[in] pvParameter Puntero a los parámetros pasados a la tarea.
 *
 * @return void
 */
void tarea_leer_tecla(void *pvParameter){
	while (1)
	{
		teclas = SwitchesRead();
		switch (teclas)
		{
		case SWITCH_1:
			encendido = true;
			break;
		case SWITCH_2:
			hold = false;
			break;
		default: 
			encendido = false;
			hold = true;
		}
		vTaskDelay(PERIODO_LECTURA_TECLAS / portTICK_PERIOD_MS);
	}
	
}


void app_main(void){

HcSr04Init(GPIO_3, GPIO_2);
LedsInit();
LedsOffAll();
LcdItsE0803Init();
SwitchesInit();
xTaskCreate(&tarea_leer_tecla, "teclas_task",512, NULL,5,NULL);
xTaskCreate(&tarea_medir, "distancia_task",2048, NULL,5,NULL);
xTaskCreate(&tarea_mostrar, "distancia_task",512, NULL,5,NULL);


}
/*==================[end of file]============================================*/