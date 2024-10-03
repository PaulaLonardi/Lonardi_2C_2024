/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * En este proyecto, se mide de distancia utilizando un sensor de ultrasonido HC-SR04. Está diseñado para medir la distancia en cm 
 * y mostrarla en el display LCD y en LEDs. 
 * Se incluyen dos botones para controlar el estado del sistema: uno para encender/apagar el sensor y otro 
 * para alternar entre mostrar la última medición y la medición en tiempo real.
 *
 * La medición se realiza a partir de dos tareas de FreeRTOS: una tarea se encarga de medir la distancia y otra de mostrarla. 
 * La tarea de medición se activa en intervalos regulares mediante un timer, asegurando una medición precisa y eficiente. 
 * La tarea de visualización actualiza tanto el estado de los LEDs como la visualización en el LCD según la distancia medida. 
 *
 * También se utiliza una aproximación de lamedición con 3 LEDs que permite indicar diferentes rangos de distancia mediante 
 * el encendido y apagado de los mismos.  
 * 
 * 
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   						|
 * |:--------------:|:----------------------------------|
 * | 	HC-SR04  	| 	GPIO_2 (Trig), GPIO_3 (Echo) 	|
 * | 	LED_1    	| 	GPIO_X        					|
 * | 	LED_2    	| 	GPIO_Y        					|
 * | 	LED_3    	| 	GPIO_Z        					|
 * | 	LCD       	| 	GPIO_A        					|
 * | 	Button 1 	| 	GPIO_B        					|
 * | 	Button 2 	| 	GPIO_C        					|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2024 | Document creation		                         |
 *
 * @author Lonardi, Paula (paula.lonardi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "hc_sr04.h"
#include "lcditse0803.h"
#include <led.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_TAREA_MEDIR_US 1000000
#define CONFIG_BLINK_PERIOD_TAREA_MOSTRAR_US 500000
/*==================[internal data definition]===============================*/
uint16_t distancia;
bool encendido;
bool hold;
TaskHandle_t handle_tarea_medir = NULL;
TaskHandle_t handle_tarea_mostrar = NULL;
/*==================[internal functions declaration]=========================*/
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
		
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */

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
		else{
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
			LcdItsE0803Off();
		}
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */


	}
	
}

void cambiar_encendido(void *pvParameter){
	encendido = !encendido;
};

void cambiar_hold(void *pvParameter){
	hold = !hold;
};

/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(handle_tarea_medir, pdFALSE);    /* Envía una notificación a la tarea medir */
}

/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR(handle_tarea_mostrar, pdFALSE);    /* Envía una notificación a la tarea mostrar */
}


/*==================[external functions definition]==========================*/
void app_main(void){

HcSr04Init(GPIO_3, GPIO_2);
LedsInit();
LedsOffAll();
LcdItsE0803Init();
SwitchesInit();

SwitchActivInt(SWITCH_1, cambiar_encendido, NULL); //configuracion de interrupcion de tecla
SwitchActivInt(SWITCH_2, cambiar_hold, NULL);

/* Inicialización de timers */
    timer_config_t timer_medir = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_TAREA_MEDIR_US,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_medir);
    timer_config_t timer_mostrar = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TAREA_MOSTRAR_US,
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_mostrar);

/* Creación de tareas */
xTaskCreate(&tarea_medir, "distancia_task",2048, NULL,5,&handle_tarea_medir);//LE PASO EL PUNTERO AL HANDLE
xTaskCreate(&tarea_mostrar, "distancia_task",512, NULL,5,&handle_tarea_mostrar);

 /* Inicialización del conteo de timers */
TimerStart(timer_medir.timer);
TimerStart(timer_mostrar.timer);

}
/*==================[end of file]============================================*/