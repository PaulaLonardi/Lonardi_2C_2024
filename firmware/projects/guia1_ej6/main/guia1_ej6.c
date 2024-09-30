/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este proyecto controla un display de 7 segmentos mediante el microcontrolador ESP32 (de la cátedra), 
 * utilizando pines GPIO para mostrar números en formato BCD. El valor numérico es convertido y mostrado demanera secuencial en el display.
 * Se configuran periféricos como los GPIO, timers y las interrupciones para los botones. 
 *
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 *  * Conexiones de hardware entre el ESP32 y el display:
 *
 * |    Peripheral  |   ESP32 Pin      |   Descripción                   		|
 * |:--------------:|:-----------------|:---------------------------------------|
 * | Segmento A     |   GPIO_20        | Control del segmento A del display 	|
 * | Segmento B     |   GPIO_21        | Control del segmento B del display 	|
 * | Segmento C     |   GPIO_22        | Control del segmento C del display 	|
 * | Segmento D     |   GPIO_23        | Control del segmento D del display 	|
 * | Dígito 1       |   GPIO_9         | Control del primer dígito del display 	|
 * | Dígito 2       |   GPIO_18        | Control del segundo dígito del display |
 * | Dígito 3       |   GPIO_19        | Control del tercer dígito del display 	|
 * 
 * Estos pines GPIO deben estar conectados a un display de 7 segmentos adecuado para que la representación BCD se muestre correctamente.
 *
 *  @note El HW incluye el display de 7 segmentos y un microcontrolador ESP32.
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                      |
 * |:----------:|:-------------------------------------------------|
 * | 12/09/2023 | Document creation		                           |
 * | 25/09/2023 | Añadida la documentación pedida en la corrección |
 * 
 * @author Lonardi, Paula (paula.lonardi@ingenieria.uner.edu.ar)
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
/**
 * @brief Convierte un número de 32 bits en su representación BCD  y la almacena en un arreglo.
 *
 * Esta función toma un número entero de 32 bits y lo descompone en sus dígitos decimales,
 * almacenando cada dígito en un arreglo proporcionado por el usuario. El arreglo de salida 
 * contendrá los dígitos en orden inverso, es decir, el dígito menos significativo estará en 
 * el primer índice del arreglo.
 *
 * @param[in] data   El número de 32 bits a convertir.
 * @param[in] digits El número de dígitos que se deben extraer del número.
 * @param[out] p_bcd_number Un puntero al arreglo donde se almacenarán los dígitos BCD.
 *                           El arreglo debe tener al menos `digits` elementos.
 *
 * @return Devuelve 1 si la conversión se realizó con éxito.
 *
 */

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
/**
 * @brief Configura los pines GPIO correspondientes para representar un valor BCD.
 *
 * Esta función toma un valor BCD de 4 bits y configura un conjunto de pines GPIO en alto o bajo 
 * dependiendo del valor de cada bit. La función inicializa los pines GPIO especificados antes de 
 * configurarlos según el valor BCD.
 *
 * @param[in] valor El valor BCD de 4 bits que se va a representar en los pines GPIO.
 * @param[in] p_vertor_pines Un puntero al arreglo de estructuras `gpioConf_t` que contiene 
 *                           la configuración de los pines GPIO. Se asume que este arreglo 
 *                           tiene al menos 4 elementos.
 *
 * @return Devuelve 1 si la operación se realizó con éxito.
 */
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


/**
 * @brief Muestra un valor numérico en un display utilizando pines GPIO.
 *
 * Esta función convierte un valor numérico de 32 bits en su representación BCD (Binary-Coded Decimal),
 * y lo muestra en un display de varios dígitos utilizando pines GPIO. Los pines GPIO son configurados
 * para seleccionar los dígitos del display y para representar cada dígito en formato BCD.
 *
 * 
 * @param[in] data El valor numérico de 32 bits que se va a mostrar en el display.
 * @param[in] digits La cantidad de dígitos que se deben mostrar.
 * @param[in] p_vertor_pines Un puntero al arreglo de estructuras `gpioConf_t` que contiene la configuración de los pines GPIO 
 *                           para representar el valor BCD. Se espera que este arreglo tenga al menos 4 elementos.
 * @param[in] p_vertor_digitos Un puntero al arreglo de estructuras `gpioConf_t` que contiene la configuración de los pines GPIO 
 *                             para seleccionar el dígito del display. Se espera que este arreglo tenga al menos 3 elementos.
 *
 * @return void
 *
 */
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
	
    // Dato a mostrar en el display
    uint32_t data = 523;

    // Mostrar el valor en el display
    mostrar_en_display(data, 3, vector_pines, vector_digitos);

   // return 0;
}
/*==================[end of file]============================================*/