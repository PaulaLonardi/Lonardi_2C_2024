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
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*==================[macros and definitions]=================================*/
#define ON 1
#define OFF 2
#define TOGGLE 3
#define CONFIG_BLINK_PERIOD 100

/*==================[internal data definition]===============================*/

struct leds
{
    uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;       // indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;   // indica el tiempo de cada ciclo
} ; 


/*==================[internal functions declaration]=========================*/

void cambiar_modo_led(struct leds* p_led){
switch(p_led->mode){
    		case ON: 
				switch (p_led->n_led)
				{
					case 1:
					LedOn(LED_1);
					printf("prendo led 1\n");
					break;
					case 2:
					LedOn(LED_2);
					break;
					case 3:
					LedOn(LED_3);
					break;
				
				default:
					break;
				}		
    		break;
    		case OFF:
				switch (p_led->n_led)
				{
					case 1:
					LedOff(LED_1);
					break;
					case 2:
					LedOff(LED_2);
					break;
					case 3:
					LedOff(LED_3);
					break;
				}

			break;
			case TOGGLE:
			printf("entro al caso toggle\n");
			uint8_t aux_retardo = p_led->periodo/100;
			switch (p_led->n_led)
						{
						case 1:
							for (size_t i = 0; i < p_led->n_ciclos; i++)
							{
								LedToggle(LED_1);	
								for (size_t j = 0; j < aux_retardo; j++)
								{
									vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
				
								}
							}
				
						break;
						case 2:
							for (size_t i = 0; i < p_led->n_ciclos; i++)
							{
								LedToggle(LED_2);
								for (size_t j = 0; j < aux_retardo; j++)
								{
									vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
				
								}
							}
						break;
						case 3:
						for (size_t i = 0; i < p_led->n_ciclos; i++)
							{
								LedToggle(LED_3);
								for (size_t j = 0; j < aux_retardo; j++)
								{
									vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
				
								}
							}
						

						break;
						}
				
			break;
		
}
}


/*==================[external functions definition]==========================*/
void app_main(void){
	
	LedsInit();

	printf("Arranca el main\n");

    // Crear una variable de tipo 'struct leds'
    struct leds led_1;

    // Inicializar los valores de la estructura
    led_1.mode = ON;       
    led_1.n_led = 1;      // LED número 2
    led_1.n_ciclos = 10;   // 5 ciclos de encendido/apagado
    led_1.periodo = 1000; // 1000 milisegundos por ciclo


// Crear una variable de tipo 'struct leds'
    struct leds led_2;

    // Inicializar los valores de la estructura
    led_2.mode = TOGGLE;       
    led_2.n_led = 2;      // LED número 2
    led_2.n_ciclos = 10;   // 5 ciclos de encendido/apagado
    led_2.periodo = 500; // 1000 milisegundos por ciclo

	struct leds led_3;

    // Inicializar los valores de la estructura
    led_3.mode = TOGGLE;       
    led_3.n_led = 3;      // LED número 2
    led_3.n_ciclos = 30;   // 5 ciclos de encendido/apagado
    led_3.periodo = 1000; // 1000 milisegundos por ciclo



	cambiar_modo_led(&led_2);
	cambiar_modo_led(&led_1);
	cambiar_modo_led(&led_3);





	printf("Hello world!\n");
}
/*==================[end of file]============================================*/