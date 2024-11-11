/*! @mainpage Sistema de Detección para Ciclistas
 *
 * @section genDesc Descripción General
 *
 * Este programa controla un dispositivo basado en ESP-EDU diseñado para mejorar la seguridad de ciclistas.
 * Incluye tres tareas que gestionan la distancia a objetos, las alarmas visuales y sonoras,
 * y la detección de caídas mediante el acelerómetro analógico.
 *
 *
 * - Indicadores LED:
 *   - LED verde para distancias > 5 metros (sin alarma sonora).
 *   - LED verde y amarillo para distancias de 3 a 5 metros, con alarma sonora de precaución.
 *   - LED verde, amarillo y rojo para distancias < 3 metros, con alarma sonora de peligro.
 *
 * - Comunicación UART:
 *   - Envía mensajes de "Peligro, vehículo cerca" y "Precaución, vehículo cerca" segun corresponda.
 *   - En caso de caída (cuando se detecta la aceleracion de 4), se envía "Caída detectada".
 *
 * Ejemplo de funcionamiento: <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Conexiones de Hardware
 *
 * 
*| HC-SR04        | ESP32               |
*|----------------|---------------------|
*| Vcc            | 5V                  |
*| Echo           | GPIO_3              |
*| Trigger        | GPIO_2              |
*| Gnd            | GND                 |
*
*| Módulo Bluetooth | ESP32            |
*|-------------------|-----------------|
*| Tx               | GPIO_16          |
*| Rx               | GPIO_17          |
*
*| Buzzer          | ESP32             |
*|-----------------|-------------------|
*| Señal           | GPIO_1            |
*| Gnd             | GND               |
*
*|acelerometro		| ESP32				|
*|-----------------	|-------------------|
*|canal_x			|CH0				|
 *
 * @section changelog Historial de Cambios
 *
 * | Fecha       | Descripción                                      |
 * |-------------|--------------------------------------------------|
 * | 04/11/2024  | Creación de la documentación                     |
 *
 * @section author Autor
 * Lonardi, Paula (paula.lonardi@ingenieria.uner.edu.ar)
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
#define CONFIG_BLINK_PERIOD_TAREA_ACELERACION_US 10000 // 100 Hz  P =  0.01s

uint16_t senial_aceleracion_x;
uint16_t senial_aceleracion_y;
uint16_t senial_aceleracion_z;

/*==================[internal data definition]===============================*/
uint16_t distancia;
TaskHandle_t handle_tarea_medir = NULL;
TaskHandle_t aceleracion_task_handle = NULL;
TaskHandle_t alarma_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

/**
 * @brief TAREA encargada de medir la distancia
 * (mide en cm, y divido por 100 para obtener el valor en metros)
 *
 * @param void
 *
 * @return void
 */

void tarea_medir(void *pvParameter)
{
	while (1)
	{
		distancia = HcSr04ReadDistanceInCentimeters() / 100;
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerA(void *param)
{
	vTaskNotifyGiveFromISR(handle_tarea_medir, pdFALSE); /* Envía una notificación a la tarea medir */
}

/**
 * @brief TAREA que gestiona la alarma sonora y  encendiendo los leds de la siguiente manera:
 * - Led verde (1) para distancias mayores a 5 metros
 * - Led verde y amarillo (2) para distancias entre 5 y 3 metros (precaucion)
 * - Led verde, amariilo y rojo (3) para distancias menores a 3 metros (peligro)
 * A su vez en situaciones de peligro y precaución, envía los mensajes requeridos mediante la UART
 *
 * @param void
 *
 * @return void
 */
void tarea_alarma(void *pvParameter)
{
	while (1)
	{
		if (distancia < 3)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);

			UartSendString(UART_CONNECTOR, "Peligro, vehículo cerca\r\n");

			GPIOOn(GPIO_1);
			vTaskDelay(250 / portTICK_PERIOD_MS);
			GPIOOff(GPIO_1); 
			vTaskDelay(250 / portTICK_PERIOD_MS);
		}
		else if (distancia > 3 && distancia < 5)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOff(LED_3);

			UartSendString(UART_CONNECTOR, "Precaución, vehículo cerca\r\n");

			GPIOOn(GPIO_1);
			vTaskDelay(500 / portTICK_PERIOD_MS);
			GPIOOff(GPIO_1); 
			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
		else if (distancia > 5)
		{
			LedOn(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
	}
	vTaskDelay(200 / portTICK_PERIOD_MS); // Pausa fuera de los condicionales
}

/**
 * @brief TAREA que gestiona el sensor de aceleración. Realiza la conversion de los valores
 * de voltaje (que llegan a partir de los canales ch0, ch1 y ch3) a G,
 * y si este valor es mayor a 4, envía el mensaje correspondiente mediante la UART
 *
 * @param void
 *
 * @return void
 */
void tarea_aceleracion(void *pvParameter)
{
	while (true)
	{

		AnalogInputReadSingle(CH0, &senial_aceleracion_x);
		AnalogInputReadSingle(CH1, &senial_aceleracion_y);
		AnalogInputReadSingle(CH3, &senial_aceleracion_z);

		float aceleracion_x = (senial_aceleracion_x / 1000.0 - 1.65) / 0.3;
		float aceleracion_y = (senial_aceleracion_y / 1000.0 - 1.65) / 0.3;
		float aceleracion_z = (senial_aceleracion_z / 1000.0 - 1.65) / 0.3;

		if ((aceleracion_x + aceleracion_y + aceleracion_z) > 4)
		{
			UartSendString(UART_CONNECTOR, "Caida detectada\r\n");
		}

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerB(void *param)
{
	vTaskNotifyGiveFromISR(aceleracion_task_handle, pdFALSE); /* Envía una notificación a la tarea aceleracion */
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	// printf("Hello world!\n");
	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
	LedsOffAll();

	GPIOInit(GPIO_1, GPIO_OUTPUT);


	// Configuración para CH0
	analog_input_config_t analog_CH0 = {
		.input = CH0, // Canal CH0
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0};
	AnalogInputInit(&analog_CH0);

	// Configuración para CH1
	analog_input_config_t analog_CH1 = {
		.input = CH1, // Canal CH1
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0};
	AnalogInputInit(&analog_CH1);

	// Configuración para CH3
	analog_input_config_t analog_CH3 = {
		.input = CH3, // Canal CH3
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0};
	AnalogInputInit(&analog_CH3);

	// Inicialización de la salida DAC
	AnalogOutputInit();

	/* Inicialización de timers */
	timer_config_t timer_medir = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_TAREA_MEDIR_US,
		.func_p = FuncTimerA,
		.param_p = NULL};
	TimerInit(&timer_medir);

	timer_config_t timer_aceleracion = {
		.timer = TIMER_B,
		.period = CONFIG_BLINK_PERIOD_TAREA_ACELERACION_US,
		.func_p = FuncTimerB,
		.param_p = NULL};
	TimerInit(&timer_aceleracion);

	serial_config_t config_serie = {
		.port = UART_CONNECTOR,
		.baud_rate = 9600,
		.func_p = NULL,
		.param_p = NULL};
	UartInit(&config_serie);

	/* Creación de tareas */
	xTaskCreate(&tarea_medir, "distancia_task", 2048, NULL, 5, &handle_tarea_medir);			  // LE PASO EL PUNTERO AL HANDLE
	xTaskCreate(&tarea_aceleracion, "aceleracion_task", 2048, NULL, 5, &aceleracion_task_handle); // LE PASO EL PUNTERO AL HANDLE
	xTaskCreate(&tarea_alarma, "alarma_task", 2048, NULL, 5, &alarma_task_handle);				  // LE PASO EL PUNTERO AL HANDLE

	/* Inicialización del conteo de timers */
	TimerStart(timer_medir.timer);
	TimerStart(timer_aceleracion.timer);
}
/*==================[end of file]============================================*/