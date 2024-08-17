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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros and definitions]=================================*/
#define DIGITOS 3

/*==================[internal data definition]===============================*/
uint8_t bcd_number[DIGITOS] = {0};


typedef struct
	{
		gpio_t pin;			/*!< GPIO pin number */
		io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
	} gpioConf_t;

gpioConf_t vector_pines[4] = {
	{GPIO_20, GPIO_OUTPUT},
	{GPIO_21, GPIO_OUTPUT},
	{GPIO_22, GPIO_OUTPUT},
	{GPIO_23, GPIO_OUTPUT}
	}; 
// defino el vector de digitos que van a ser mostrados en el display
gpioConf_t vector_digitos[3] = {
	{GPIO_9, GPIO_OUTPUT}, 
	{GPIO_18, GPIO_OUTPUT}, 
	{GPIO_19, GPIO_OUTPUT} 
};

/*==================[internal functions declaration]=========================*/

//funcion del ejercicio 4
uint8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t *p_bcd_number)
{
	for (size_t i = 0; i < digits; i++)
	{
		p_bcd_number[i] = data %10;
		data = data /10;
		printf("indice %d, valor %d \r\n",i, p_bcd_number[i]);
	}
	return 1;
}


//funcion del ejercicio 5
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



	void mostrar_en_display(uint32_t data, uint8_t digits, gpioConf_t *p_vertor_pines, gpioConf_t *p_vertor_digitos)
{
    uint8_t bcd_number[digits];
			//inicializo los gpio con su numero de pin y 
	for (size_t i = 0; i < 3; i++)
		{
			GPIOInit(p_vertor_digitos[i].pin, p_vertor_digitos[i].dir);
		}
	

    // Convertir el dato a BCD
    convertToBcdArray(data, digits, bcd_number);

    // Mostrar cada dígito en su respectivo lugar en el display
    for (size_t i = 0; i < digits; i++)
    {
		// Configurar los pines GPIO para mostrar el dígito actual
        BCD_a_GPIO(bcd_number[i], p_vertor_pines);

        // Seleccionar el dígito a mostrar
        GPIOOn(p_vertor_digitos[i].pin);  // Activar el dígito correspondiente
		printf("indice del digito en on %d, valor %d \r\n",p_vertor_digitos[i].pin, bcd_number[i]);

        // Desactivar el dígito después de mostrarlo
        GPIOOff(p_vertor_digitos[i].pin);
		printf("indice del digito en off %d, valor %d \r\n",i, bcd_number[i]);
        vTaskDelay(1000/ portTICK_PERIOD_MS);

    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
	//printf("Hello world!\n");
	
	


    // Dato a mostrar en el display
    uint32_t data = 523;

    // Mostrar el valor en el display
    mostrar_en_display(data, 3, vector_pines, vector_digitos);

   // return 0;
}
/*==================[end of file]============================================*/