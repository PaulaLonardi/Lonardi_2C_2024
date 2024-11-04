/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>

#include <stdbool.h>

#include "hc_sr04.h"
#include "gpio_mcu.h"

#include <led.h>

#include "timer_mcu.h"
#include "uart_mcu.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "analog_io_mcu.h"

#include "buzzer.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_TAREA_MEDIR_US 500000
#define CONFIG_BLINK_PERIOD_TAREA_ACELERACION_US 0.1 //100 HZ

uint16_t senial_aceleracion_x;
uint16_t senial_aceleracion_y;
uint16_t senial_aceleracion_z;

/*==================[internal data definition]===============================*/
uint16_t distancia;
TaskHandle_t handle_tarea_medir = NULL;
TaskHandle_t aceleracion_task_handle = NULL;
/*==================[internal functions declaration]=========================*/


/**
 * @brief TAREA encargada de medir la distancia 
 * (mide en cm, y multiplico por 100 para obtener el valor en metros)
 *
 * @param void
 *
 * @return void
 */

void tarea_medir(void *pvParameter){
	while (1)
	{ 
		distancia = 100 * HcSr04ReadDistanceInCentimeters();
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   
	}
}

/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(handle_tarea_medir, pdFALSE);    /* Envía una notificación a la tarea medir */
}

/**
 * @brief TAREA que gestiona la alarma, encendiendo los leds de la siguiente manera
 * Led verde para distancias mayores a 5 metros
 * Led verde y amarillo para distancias entre 5 y 3 metros (precaucion)
 * Led verde, amariilo y rojo para distancias menores a 3 metros (peligro)
 *
 * @param void
 *
 * @return void
 */
void tarea_alarma(void *pvParameter){
	while (1)
	{ 
		if (distancia < 3)
			{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);

			UartSendString(UART_CONNECTOR, "Peligro, vehículo cerca\r\n");

			GPIOOn(GPIO_2);
			vTaskDelay(500/portTICK_PERIOD_MS);
			GPIOOff(GPIO_2);
			}
			else if(distancia > 3 && distancia < 5)
			{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOff(LED_3);

			UartSendString(UART_CONNECTOR, "Precaución, vehículo cerca\r\n");

			GPIOOn(GPIO_2);
			vTaskDelay(1000/portTICK_PERIOD_MS);
			GPIOOff(GPIO_2);
			}
			else if(distancia > 5)
			{
			LedOn(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
			}
		
	}
}

void tarea_aceleracion(void *pvParameter){
	while(true){

    	AnalogInputReadSingle(CH0, &senial_aceleracion_x);
		AnalogInputReadSingle(CH1, &senial_aceleracion_y);
		AnalogInputReadSingle(CH3, &senial_aceleracion_z);

		if((senial_aceleracion_x + senial_aceleracion_y + senial_aceleracion_z)*(5.5/3.3) > 4)
		{
			UartSendString(UART_CONNECTOR, "Caida detectada\r\n");
		}

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 	
        vTaskNotifyGiveFromISR(aceleracion_task_handle, pdFALSE); 

    }
}

/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void* param){
    vTaskNotifyGiveFromISR(aceleracion_task_handle, pdFALSE);    /* Envía una notificación a la tarea aceleracion */
}


/*==================[external functions definition]==========================*/
void app_main(void){
	//printf("Hello world!\n");
	HcSr04Init(GPIO_3, GPIO_1);
	LedsInit();
	LedsOffAll();

    GPIOInit(GPIO_2,GPIO_OUTPUT);
    BuzzerInit(GPIO_2);

	/* Inicialización de timers */
    timer_config_t timer_medir = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_TAREA_MEDIR_US,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_medir);

	timer_config_t timer_aceleracion = {
			.timer = TIMER_B,
			.period = CONFIG_BLINK_PERIOD_TAREA_ACELERACION_US,
			.func_p = FuncTimerB,
			.param_p = NULL
		};
    TimerInit(&timer_aceleracion);

	serial_config_t config_serie = {			
		.port = UART_CONNECTOR,	
		.baud_rate = 9600,		
		.func_p = NULL,
		.param_p = NULL
	};
	UartInit(&config_serie);

/* Creación de tareas */
xTaskCreate(&tarea_medir, "distancia_task",2048, NULL,5,&handle_tarea_medir);//LE PASO EL PUNTERO AL HANDLE
xTaskCreate(&tarea_medir, "aceleracion_task",2048, NULL,5,&aceleracion_task_handle);//LE PASO EL PUNTERO AL HANDLE

 /* Inicialización del conteo de timers */
TimerStart(timer_medir.timer);
TimerStart(timer_aceleracion.timer);




}
/*==================[end of file]============================================*/