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
#include <math.h>
/*==================[macros and definitions]=================================*/
#define DIGITOS 3
/*==================[internal data definition]===============================*/
uint8_t bcd_number[DIGITOS] = {0};
/*==================[internal functions declaration]=========================*/

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


/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");

	uint8_t dato = 123;
	
    printf("Representacion en BCD: ");
	convertToBcdArray(dato, DIGITOS, bcd_number);
    for (int i = DIGITOS - 1; i >= 0; i--)  
    {
        printf("%d", bcd_number[i]);
    }
    printf("\n");

}
/*==================[end of file]============================================*/