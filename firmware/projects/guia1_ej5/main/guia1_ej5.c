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
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
	{
		gpio_t pin;			/*!< GPIO pin number */
		io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
	} gpioConf_t;


gpioConf_t vector_pines[4] = {{GPIO_20, GPIO_OUTPUT},{GPIO_21, GPIO_OUTPUT},{GPIO_22, GPIO_OUTPUT},{GPIO_23, GPIO_OUTPUT}}; 
//vector de estructuras tipo gpioconft

/*==================[internal functions declaration]=========================*/
uint8_t BCD_a_GPIO(uint8_t valor, gpioConf_t *p_vertor_pines)
	{
		uint8_t mask = 1; //mascara que se usa en la comparacion
		
		
		//inicializo los gpio con su numero de pin y 
		for (size_t i = 0; i < 4; i++)
		{
			GPIOInit(p_vertor_pines[i].pin, p_vertor_pines[i].dir);
		}
		
		//recorro el valor para ver cual está en 0 o 1 y pongo la pata en bajo o alto respectivamente 
		for (size_t j = 0; j < 4; j++)
		{
			if ((mask & valor) != 0){
			// si el valor es distinto de 0 (puede ser 1,2, etc) va a poner la patita en alto, sino, pone la patita en bajo
			
				GPIOOn(p_vertor_pines[j].pin);
			} else{
				GPIOOff(p_vertor_pines[j].pin);
			}
			mask = mask << 1;
		}
		
		return 1;
	}
/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");

	BCD_a_GPIO(5, vector_pines);

}
/*==================[end of file]============================================*/