/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * El programa responde a la consigna del recuperatorio del parcial de electronica programable
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 * 
 *| HC-SR04        | ESP32               |
 *|----------------|---------------------|
 *| Vcc            | 5V                  |
 *| Echo           | GPIO_3              |
 *| Trigger        | GPIO_2              |
 *| Gnd            | GND                 |
 *
 *| Barrera         | ESP32             |
 *|-----------------|-------------------|
 *| Señal           | GPIO_1            |
 *| Gnd             | GND               |

 *| Balanza         | ESP32             |
 *|-----------------|-------------------|
 *| g1           	| CH1            	|
 *| g2             	| CH2               |

 *
 * @section changelog Historial de Cambios
 *
 * | Fecha       | Descripción                                      |
 * |-------------|--------------------------------------------------|
 * | 11/11/2024  | Creación de la documentación                     |
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
#define CONFIG_BLINK_PERIOD_TAREA_MEDIR_US 100000 // 10hz
#define CONFIG_BLINK_PERIOD_PESAR_US 5000		  // 200Hz

/*==================[internal data definition]===============================*/
uint16_t distancia;
uint16_t peso_g1;
uint16_t peso_g2;
float promedio_peso;
float velocidad;
float velocidad_maxima;
bool pesaje = false;

TaskHandle_t handle_tarea_medir = NULL;
TaskHandle_t handle_tarea_pesaje = NULL;

/*==================[internal functions declaration]=========================*/
/**
 * @brief TAREA que mide la distancia utilizando el sensor ultrasónico HC-SR04.
 *
 * - La tarea mide y guarda la distancia utilizando la función "HcSr04ReadDistanceInCentimeters()", y luego pasandolo a m.
 * La tarea entra en espera utilizando "ulTaskNotifyTake()" hasta que recibe una notificación para volver a ejecutar 
 * el proceso de medición.
 * - Mide la velocidad, la guarda y tambien mide la velocidad maxima 
 * - Se encarga del encendido y apagado de los leds correspondientes segun la distancia
 *
 * @param[in] pvParameter Puntero a los parámetros pasados a la tarea.
 *
 * @return void
 */
void tarea_medir(void *pvParameter)
{
	uint16_t distancia_anterior = 10;
	velocidad_maxima = 0;
	while (1)
	{
		distancia_anterior = distancia;
		distancia = HcSr04ReadDistanceInCentimeters() / 100; // lo paso a metros
		//distancia_anterior = distancia;
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */

		// pesaje = false;

		if (distancia < 10)
		{
			// calculo la velocidad
			velocidad = (distancia_anterior - distancia) / 0.1;

			if (velocidad > 8) // velocidad mayor a 8m/s: LED3,
			{
				LedOff(LED_1);
				LedOff(LED_2);
				LedOn(LED_3);
				pesaje = false;
				if (velocidad > velocidad_maxima)
				{
					velocidad_maxima = velocidad;
				}
			}
			else if (velocidad < 8 && velocidad > 0) // velocidad entre 0m/s y 8m/s: LED2,
			{
				LedOff(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
				pesaje = false;

				if (velocidad > velocidad_maxima)
				{
					velocidad_maxima = velocidad;
				}
			}
			if (velocidad == 0) // vehículo detenido:LED1.
			{
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);

				pesaje = true;
			}
		}
	}
}


/**
 * @brief Esta tarea se encarga de realizar el pesaje del camion cuando este se encuentra en estado de reposo.
 * Para calcular el peso, se toma un promedio de 50 muestras de la suma de ambas galgas.
 * Luego, se encarga de informar mediante la uart el valor del peso y la velocidad maxima que este tuvo durante el recorrido
 * @param void 
 *
 * @return void
 */

void tarea_pesar(void *pvParameter)
{
	uint16_t peso_anterior_g1 = 0;
	uint16_t peso_anterior_g2 = 0;

	uint16_t iteracion;
	iteracion = 1;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */

		if (pesaje)
		{
			AnalogInputReadSingle(CH1, &peso_g1);
			AnalogInputReadSingle(CH2, &peso_g2);
			iteracion++;

			if (iteracion <= 50)
			{
				peso_g1 = peso_g1 + peso_anterior_g1;
				peso_g2 = peso_g2 + peso_anterior_g2;
			}
			else
			{
				promedio_peso = (peso_g1 + peso_g2) / 50;
				iteracion = 0;
				UartSendString(UART_PC, "Peso: ");
				UartSendString(UART_PC, (const char *)UartItoa(promedio_peso, 10));
				UartSendString(UART_PC, " kg\r\n");

				UartSendString(UART_PC, "Velocidad maxima: ");
				UartSendString(UART_PC, (const char *)UartItoa(velocidad_maxima, 10));
				UartSendString(UART_PC, " m/s\r\n");
			}
		}
	}
}



/**
 * @brief Esta funcion se encarga de poner en alto el gpio correspondiente a la barrera
 * @param void 
 *
 * @return void
 */
void abrir_barrera()
{
	GPIOOn(GPIO_1);
};
/**
 * @brief Esta funcion se encarga de poner en bajo el gpio correspondiente a la barrera
 * @param void 
 *
 * @return void
 */
void cerrar_barrera()
{
	GPIOOff(GPIO_1);
};

/**
 * @brief Lee una tecla ingresada desde la terminal y ejecuta el par de acciones basadas en el carácter recibido.
 *
 * Esta función lee un byte desde el puerto UART y, dependiendo del valor de la tecla ingresada, realiza una de las dos acciones:
 * Si se ingresa 'O', se abre la barrera.
 * Si se ingresa 'C', se cierra la barrera.
 *
 * @param void 
 *
 * @return void
 */
void leer_teclado()
{
	uint8_t letra;
	UartReadByte(UART_PC, &letra);
	switch (letra)
	{
	case 'O':
		abrir_barrera();
		break;
	case 'C':
		cerrar_barrera();
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
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerB(void *param)
{
	vTaskNotifyGiveFromISR(handle_tarea_pesaje, pdFALSE); /* Envía una notificación a la tarea medir */
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	// printf("Hello world!\n");
	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
	LedsOffAll();

	GPIOInit(GPIO_1, GPIO_OUTPUT);

	/*Inicializo los timers*/
	timer_config_t timer_medir = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_TAREA_MEDIR_US,
		.func_p = FuncTimerA,
		.param_p = NULL};
	TimerInit(&timer_medir);

	timer_config_t timer_pesar = {
		.timer = TIMER_B,
		.period = CONFIG_BLINK_PERIOD_PESAR_US,
		.func_p = FuncTimerB,
		.param_p = NULL};
	TimerInit(&timer_pesar);

	// Configuración para CH1
	analog_input_config_t analog_CH1 = {
		.input = CH1, // Canal CH1
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0};
	AnalogInputInit(&analog_CH1);

	// Configuración para CH3
	analog_input_config_t analog_CH2 = {
		.input = CH2, // Canal CH2
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0};
	AnalogInputInit(&analog_CH2);


	serial_config_t config_serie = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = leer_teclado,
		.param_p = NULL};
	UartInit(&config_serie);

	/* Creación de tareas */
	xTaskCreate(&tarea_medir, "distancia_task", 2048, NULL, 5, &handle_tarea_medir);	// LE PASO EL PUNTERO AL HANDLE
	xTaskCreate(&tarea_pesar, "aceleracion_task", 2048, NULL, 5, &handle_tarea_pesaje); // LE PASO EL PUNTERO AL HANDLE

	/* Inicialización del conteo de timers */
	TimerStart(timer_medir.timer);
	TimerStart(timer_pesar.timer);
}
/*==================[end of file]============================================*/