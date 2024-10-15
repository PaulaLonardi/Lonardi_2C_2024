/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 * 
 * En este proyecto, se mide de distancia utilizando un sensor de ultrasonido HC-SR04. 
 * La distancia medida se muestra en una terminal serie, en un display LCD y mediante LEDs que indican 
 * diferentes rangos de distancia. El sistema se controla mediante dos botones o mediante la letra O y H, uno para encender o 
 * apagar el sistema y otro para mantener la última medición mostrada. La medición de distancia se 
 * realiza periódicamente utilizando tareas de FreeRTOS para asegurar una respuesta rápida y eficiente del sistema.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   		|
 * |:--------------:|:--------------	|
 * | 	HC-SR04  	| 	GPIO_2, GPIO_3	|
 * | 	LED_1		| 	GPIO_X			|
 * | 	LED_2		| 	GPIO_Y			|
 * | 	LED_3		| 	GPIO_Z			|
 * | 	LCD         | 	GPIO_A, GPIO_B	|
 * | 	SWITCH_1	| 	GPIO_C			|
 * | 	SWITCH_2	| 	GPIO_D			|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 20/09/2024 | Document creation		                         |
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
#include "uart_mcu.h"
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


void cambiar_encendido(){
	encendido = !encendido;
};

void cambiar_hold(){
	hold = !hold;
};



/**
 * @brief Muestra la distancia actual en la terminal serie.
 *
 * Convierto el valor de la variable global "distancia" a una cadena de caracteres, después lo paso a formato decimal
 * y la envía a través del puerto UART junto "cm" para su visualización en la terminal.
 *
 * @param void 
 *
 * @return void 
 *
 */
void mostrar_en_terminal(){
	UartSendString(UART_PC, (const char*)UartItoa(distancia, 10));
	UartSendString(UART_PC, " cm\r\n");
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
 * @brief Lee una tecla ingresada desde la terminal y ejecuta el par de acciones basadas en el carácter recibido.
 *
 * Esta función lee un byte desde el puerto UART y, dependiendo del valor de la tecla ingresada, realiza una de las dos acciones:
 * Si se ingresa 'O', se llama a la función cambiar_encendido().
 * Si se ingresa 'H', se llama a la función cambiar_hold().
 *
 * @param void 
 *
 * @return void
 */

void leer_teclado(){
	uint8_t letra;
	UartReadByte(UART_PC, &letra);
	switch (letra)
	{
	case 'O':
		cambiar_encendido();
		break;
	case 'H':
		cambiar_hold();
	
	}
}



/**
 * @brief TAREA encargada de mostrar la mediciónmdiante LEDs, terminal y un display LCD.
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
			mostrar_en_terminal();
			
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

serial_config_t config_serie = {			
	.port = UART_PC,	
	.baud_rate = 9600,		
	.func_p = leer_teclado,
    .param_p = NULL
};
UartInit(&config_serie);


/* Creación de tareas */
xTaskCreate(&tarea_medir, "distancia_task",2048, NULL,5,&handle_tarea_medir);//LE PASO EL PUNTERO AL HANDLE
xTaskCreate(&tarea_mostrar, "distancia_task",512, NULL,5,&handle_tarea_mostrar);

 /* Inicialización del conteo de timers */
TimerStart(timer_medir.timer);
TimerStart(timer_mostrar.timer);

}
/*==================[end of file]============================================*/