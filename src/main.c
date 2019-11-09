//Project main source file
//Group: 021
//Project: Graphing Calculator w/ Cortex-m0
//Members: <insert our names>
//PUIDs: <insert IDs>

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

void serial_init(void);

int main(void)
{
    //Add some serial initialization for the terminal communication
    serial_init();

    //Change these tests as needed inside the other source files (eg. calculations.c, or parsing.c)
    //Don't change this file too much so that that putting our blocks together
    //is easier after we finish our unit tests
	calculatorTest();
	//lcdTest();
	//parsingTest();
}

void serial_init(void){
    //Can copy this from the HMWK (either 4, 5, or 6) where we did the serial-to-terminal
    //Do this

}
