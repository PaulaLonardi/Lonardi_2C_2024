/*! @mainpage Proyecto de lectura de datos y conversion AD y DA
 *
 * @section genDesc General Description
 *
 *  Este programa está diseñado para medir señales analógicas desde un canal, 
 *  convertirlas a valores digitales y enviarlas mediante UART.
 *  Por otra parte, genera una señal de ECG simulada con los datos otorgados por la cátedra,
 *  que es enviada como salida analógica.
 * En este proyecto se realiza tanto conversión analógica-digital como digital-analógica
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	UART_PC		| 	GPIO_X		|
 * | 	CH1			| 	GPIO_Y		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 03/10/2024 | Document creation		                         |
 *
 * @author Lonardi, Paula (paula.lonardi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <led.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_TAREA_MEDIR_Y_ENVIAR_US 20000
#define CONFIG_BLINK_PERIOD_ECG 10000
#define TAMANIO_DE_BUFER 231

/*==================[internal data definition]===============================*/
/**
 * @brief Manejadores de las tareas de FreeRTOS.
 */
TaskHandle_t handle_tarea_medir_enviar = NULL;
TaskHandle_t handle_main = NULL;

/**
 * @brief Buffer con datos de una simulacion de ECG
 */
uint8_t ecg[TAMANIO_DE_BUFER] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};


/*==================[internal functions declaration]=========================*/

/**
 * @brief Tarea de medición y envío de datos.
 * 
 * Esta TAREA mide continuamente un valor analógico desde el canal CH1, lo convierte a un valor 
 * digital y lo envía mediante UART. 
 * La tarea espera en un bucle hasta recibir una notificación para continuar.
 *
 * @param[in] pvParameter Parámetro de entrada para la tarea (no lo utilizo).
 * 
 * El funcionamiento de la tarea es el siguiente:
 * - Lee un valor analógico usando `AnalogInputReadSingle`.
 * - Convierte el valor a cadena con `UartItoa` y lo envía mediante UART usando `UartSendString`.
 * - Espera una notificación mediante `ulTaskNotifyTake` antes de continuar con la siguiente medición.
 */
void tarea_medir_enviar(void *pvParameter){
	uint16_t valor_medida;
	while (1)
	{ 
		AnalogInputReadSingle(CH1, &valor_medida);//lee y devuelve el valor digitalizado
		UartSendString(UART_PC, (const char*)UartItoa(valor_medida, 10)); // Paso la cadena a un valor y envío a través de UART
		UartSendString(UART_PC, "\r");
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // La tarea espera en este punto hasta recibir una notificación
	}
}

/**
 * @brief Tarea de salida de datos de ECG.
 * 
 * Simula el comportamiento de una señal ECG, enviando valores almacenados en un búfer a la salida 
 * analógica de manera continua.
 * Recorre los valores del búfer ecg[] (otorgado por la cátedra) y los envía utilizando la función 
 * AnalogOutputWrite. Cuando llega al final del búfer, vuelve a empezar desde el inicio.
 * La tarea espera en un bucle hasta recibir una notificación antes de procesar el siguiente valor.
 * 
 * @param[in] pvParameter Parámetro de entrada para la tarea (no lo utilizo).
 * 
 */
void tarea_ecg(void *pvParameter)
{
	uint8_t i = 0;
	while(1){			
		if(i < TAMANIO_DE_BUFER){
			AnalogOutputWrite(ecg[i]);
			i++;
		}
		else i = 0;
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
	
	}
}


/**
 * @brief Función llamada de manera automática por el temporizador A.
 * 
 * Esta función es llamada por el temporizador A cuando se cumple el período configurado. 
 * Envía una notificación a la tarea tarea_medir_enviar para que continúe su ejecución.
 * 
 * @param[in] param Parámetro de entrada para la función (no lo utilizo).
 * 
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(handle_tarea_medir_enviar, pdFALSE);    /* Envía una notificación a la tarea medir */
}

/**
 * @brief Función llamada de manera automática por el temporizador B.
 * 
 * Esta función es llamada por el temporizador B cuando se cumple el período configurado. 
 * Envía una notificación a la tarea principal handle_main para que continúe su ejecución.
 * 
 * @param[in] param Parámetro de entrada para la función (no lo utilizo).
 * 
 */
void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR(handle_main, pdFALSE);    /* Envía una notificación a la tarea medir */
}



/*==================[external functions definition]==========================*/

/**
 * @brief Función principal de la aplicación.
 *
 * Inicializa UART, entradas y salidas analógicas, timers y crea las tareas.
 */
void app_main(void){

	serial_config_t my_uart = {
		.port = UART_PC, 
		.baud_rate = 38400, //(1/38400)*50 < 20ms de muestreo
		.func_p = NULL, 
		.param_p = NULL
	};

	UartInit(&my_uart);

	analog_input_config_t analog = {
		.input = CH1, //le paso el canal
		.mode = ADC_SINGLE,//el modo en el que va a operar
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0
	};

	AnalogInputInit(&analog);
	AnalogOutputInit();


	/* Inicialización de timers */
    timer_config_t timer_medir_y_enviar = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_TAREA_MEDIR_Y_ENVIAR_US,
        .func_p = FuncTimerA,
        .param_p = NULL
    };

    TimerInit(&timer_medir_y_enviar);


	/* Inicialización del timer out */
    timer_config_t timer_ecg = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_ECG,
        .func_p = FuncTimerB,
        .param_p = NULL
    };

    TimerInit(&timer_ecg);

	xTaskCreate(&tarea_medir_enviar, "medicion",2048, NULL,5,&handle_tarea_medir_enviar);
	xTaskCreate(&tarea_ecg, "tarea_ECG",2048, NULL,5,&handle_main);

	TimerStart(timer_medir_y_enviar.timer);
	TimerStart(timer_ecg.timer);
}
/*==================[end of file]============================================*/