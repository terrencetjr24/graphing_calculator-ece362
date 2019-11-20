// (c) Rados≥aw KwiecieÒ, radek@dxp.pl
// http://en.radzio.dxp.pl
//-------------------------------------------------------------------------------------------------
#include "KS0108.h"
#include "graphic.h"
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <math.h>
#include "parsing.h"

#define GPIO_ODR_0_D0                 ((uint32_t)0x00000001)
#define GPIO_ODR_1_D1                 ((uint32_t)0x00000002)
#define GPIO_ODR_2_D2                 ((uint32_t)0x00000004)
#define GPIO_ODR_3_D3                 ((uint32_t)0x00000008)
#define GPIO_ODR_4_D4                 ((uint32_t)0x00000010)
#define GPIO_ODR_5_D5                 ((uint32_t)0x00000020)
#define GPIO_ODR_6_D6                 ((uint32_t)0x00000040)
#define GPIO_ODR_7_D7                 ((uint32_t)0x00000080)
#define GPIO_ODR_8_CS1                 ((uint32_t)0x00000100)
#define GPIO_ODR_9_CS2                 ((uint32_t)0x00000200)
#define GPIO_ODR_10_RW                ((uint32_t)0x00000400)
#define GPIO_ODR_11_RS                ((uint32_t)0x00000800)
#define GPIO_ODR_12_EN                ((uint32_t)0x00001000)

void micro_wait(unsigned int);

// Start of keypad setup
int col = 0;
int8_t history[16] = {0};
int8_t lookup[16] = {1,4,7,0xe,2,5,8,0,3,6,9,0xf,0xa,0xb,0xc,0xd};
char char_lookup[16] = {'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'};
char screen[2][8][64] = {0};
//char screen[2][64][8] = {0};

void on(int x, int y) {
	int half = x >> 6;
	int row = y / 8;
	//int row = y;
	int col = x % 64;
	//int col = x / 8;

	/*int c = col;
	char snum[10] = {0};
	itoa(c, snum, 10);
	GLCD_GoTo(0,0);
	GLCD_WriteString(snum);
	screen[half][row][col] |= 1 << (y % 8);*/

	pixel_to_screen(half, row, col);
}

void off(int x, int y) {
	int half = x >> 6;
	int row = y /8;
	int col = x % 64;

	screen[half][row][col] &= ~(1 << (y % 8));

	pixel_to_screen(half, row, col);
}

void pixel_to_screen(int half, int row, int col) {
	GPIOA->ODR &= ~(GPIO_ODR_10_RW | GPIO_ODR_11_RS);

	GPIOA->ODR &= ~(GPIO_ODR_8_CS1 | GPIO_ODR_9_CS2);
	if(half == 0)
	{
		GPIOA->ODR |= GPIO_ODR_8_CS1;
	}
	else
	{
		GPIOA->ODR |= GPIO_ODR_9_CS2;
	}

	/*GPIOA->ODR |= row | 0xb8; //setting x address
	GPIOA->ODR |= GPIO_ODR_12_EN;
	micro_wait(10);
	GPIOA->ODR &= ~GPIO_ODR_12_EN;
	micro_wait(10);

	GPIOA->ODR |= col | 0x40; //setting y address
	GPIOA->ODR |= GPIO_ODR_12_EN;
	micro_wait(10);
	GPIOA->ODR &= ~GPIO_ODR_12_EN;
	micro_wait(10);*/

	GPIOA->ODR |= row | 0xb8; //setting y address
	GPIOA->ODR |= GPIO_ODR_12_EN;
	micro_wait(10);
	GPIOA->ODR &= ~GPIO_ODR_12_EN;
	micro_wait(10);

	GPIOA->ODR |= col | 0x40; //setting x address
	GPIOA->ODR |= GPIO_ODR_12_EN;
	micro_wait(10);
	GPIOA->ODR &= ~GPIO_ODR_12_EN;
	micro_wait(10);

	GPIOA->ODR |= GPIO_ODR_11_RS;
	GPIOA->ODR |= screen[half][row][col];
	GPIOA->ODR |= GPIO_ODR_12_EN;
	micro_wait(10);
	GPIOA->ODR &= ~GPIO_ODR_12_EN;
	micro_wait(10);

	/*char c = GPIOA->ODR;
	char snum[10] = {0};
	itoa(c, snum, 10);
	GLCD_GoTo(0,2);
	GLCD_WriteString(&c);*/

}

void setup_gpio() {
    /* Student code goes here */
	RCC-> AHBENR |= RCC_AHBENR_GPIOAEN;

	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOA->MODER &= ~GPIO_MODER_MODER1;
	GPIOA->MODER &= ~GPIO_MODER_MODER2;
	GPIOA->MODER &= ~GPIO_MODER_MODER3;
	GPIOA->MODER &= ~GPIO_MODER_MODER4;
	GPIOA->MODER &= ~GPIO_MODER_MODER5;
	GPIOA->MODER &= ~GPIO_MODER_MODER6;
	GPIOA->MODER &= ~GPIO_MODER_MODER7;
	GPIOA->MODER &= ~GPIO_MODER_MODER8;
	GPIOA->MODER &= ~GPIO_MODER_MODER9;
	GPIOA->MODER &= ~GPIO_MODER_MODER10;
	GPIOA->MODER &= ~GPIO_MODER_MODER11;
	GPIOA->MODER &= ~GPIO_MODER_MODER12;

	GPIOA->MODER |= GPIO_MODER_MODER0_0;
	GPIOA->MODER |= GPIO_MODER_MODER1_0;
	GPIOA->MODER |= GPIO_MODER_MODER2_0;
	GPIOA->MODER |= GPIO_MODER_MODER3_0;
	GPIOA->MODER |= GPIO_MODER_MODER4_0;
	GPIOA->MODER |= GPIO_MODER_MODER5_0;
	GPIOA->MODER |= GPIO_MODER_MODER6_0;
	GPIOA->MODER |= GPIO_MODER_MODER7_0;
	GPIOA->MODER |= GPIO_MODER_MODER8_0;
	GPIOA->MODER |= GPIO_MODER_MODER9_0;
	GPIOA->MODER |= GPIO_MODER_MODER10_0;
	GPIOA->MODER |= GPIO_MODER_MODER11_0;
	GPIOA->MODER |= GPIO_MODER_MODER12_0;
}

int get_key_pressed() {
	int key = get_key_press();
	while(key != get_key_release());
	return key;
}

char get_char_key() {
	int index = get_key_pressed();
	return char_lookup[index];
}

// This function should enable the clock to port B, configure pins 0, 1, 2 and
// 3 as outputs (we will use these to drive the columns of the keypad).
// Configure pins 4, 5, 6 and 7 to have a pull down resistor
// (these four pins connected to the rows will being scanned
// to determine the row of a button press).
void init_keypad() {
	/* Student code goes here */
	RCC-> AHBENR |= RCC_AHBENR_GPIOBEN;

	// Setting PA0,PA1,PA2,PA3 to Output mode
	GPIOB->MODER &= ~GPIO_MODER_MODER0;
	GPIOB->MODER &= ~GPIO_MODER_MODER1;
	GPIOB->MODER &= ~GPIO_MODER_MODER2;
	GPIOB->MODER &= ~GPIO_MODER_MODER3;

	GPIOB->MODER |= 0x55; // output mode

	GPIOB->MODER &= ~GPIO_MODER_MODER4;
	GPIOB->MODER &= ~GPIO_MODER_MODER5;
	GPIOB->MODER &= ~GPIO_MODER_MODER6;
	GPIOB->MODER &= ~GPIO_MODER_MODER7;


	GPIOB->PUPDR &= ~GPIO_MODER_MODER4;
	GPIOB->PUPDR &= ~GPIO_MODER_MODER5;
	GPIOB->PUPDR &= ~GPIO_MODER_MODER6;
	GPIOB->PUPDR &= ~GPIO_MODER_MODER7;

	GPIOB->PUPDR |= 0xAA00;
}

// This function should,
// enable clock to timer6,
// setup pre scalar and arr so that the interrupt is triggered every
// 1ms, enable the timer 6 interrupt, and start the timer.
void setup_timer6() {
    /* Student code goes here */
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	TIM6->ARR = 47;
	TIM6->PSC = 999;
	TIM6->DIER |= TIM_DIER_UIE;
	TIM6->CR1 |= TIM_CR1_CEN;

	NVIC->ISER[0] = 1 <<TIM6_DAC_IRQn;

}

// The functionality of this subroutine is described in the lab document
int get_key_press() {
	/* Student code goes here */
	while(1)
	{
	 for(int i = 0; i < 16; i++)
	 {
		 if (history[i] == 1)
			 return i;
	 }
	 }
	return 0;
}

// The functionality of this subroutine is described in the lab document
int get_key_release() {
	/* Student code goes here */
	while(1) {
	 for(int i = 0; i < 16; i++) {
		if (history[i] == -2)
			return i;
	 }
	 }
	return 0;
}

// See lab document for the instructions as to how to fill this
void TIM6_DAC_IRQHandler() {
	/* Student code goes here */
	TIM6->SR &= ~TIM_SR_UIF;

	int row = (GPIOB->IDR >> 4) & 0xf;
	int index = col<<2;
	history[index] = history[index]<<1;
	history[index] |= (row & 0x1);

	history[index+1] = history[index+1]<<1;
	history[index+2] = history[index+2]<<1;
	history[index+3] = history[index+3]<<1;

	history[index+1] |= ((row>>1) & 0x1);
	history[index+2] |= ((row>>2) & 0x1);
	history[index+3] |= ((row>>3) & 0x1);

	col = col + 1;
	if (col > 3)
	{
		col = 0;
	}

	GPIOB->ODR = 1<<col;

}
// End of keypad set-up


void init_hardware(void)
{
    GLCD_Initalize();
    GLCD_ClearScreen();

    //GLCD_GoTo(0,0);
    //GLCD_WriteString(keys);
    //init_lcd();
    init_keypad();
    setup_timer6();
    setup_gpio();

    //display1("Problem 3");
    //display2(keys);

}

int main(void)
{
	init_hardware();
	init_stack(buttonStack);

	/*for(int i = 0; i < 128; i++) {
		on(i,10);
	}*/

	int h = 0;
	int r = 4;
	int c = 10;
	pixel_to_screen(0, 4, 10);

    /*char keys[16] = {"Key Pressed:"};

    while(1) {
        char key = get_char_key();
        if(key != '\0') {
            keys[12] = key;
            GLCD_GoTo(0,0);
            GLCD_WriteString(keys);
        }
    }*/
/*
GLCD_Initalize();
GLCD_ClearScreen();

GLCD_GoTo(0,0);
GLCD_WriteString("+-------------------+");
GLCD_GoTo(0,1);
GLCD_WriteString("Calculator is dumb");
GLCD_GoTo(0,2);
GLCD_WriteString("Why is this so dull");
GLCD_GoTo(0,3);
GLCD_WriteString("There are only 8 rows");
GLCD_GoTo(0,4);
GLCD_WriteString("How do I type the division sign");
GLCD_GoTo(0,5);
GLCD_WriteString("+-=*^()");
GLCD_GoTo(0,6);
GLCD_WriteString("Max 21 characters");
GLCD_GoTo(0,7);
GLCD_WriteString("012345678901234567891");
*/
for( ; ; );
return 0;
}

double factorial(double input){
    double output = 1;
    int i;
    for(i=1; i<= input; i++)
        output *= i;

    return output;
}

double radToDeg(double input){
  double output = (input * 180.0)/3.14159;
  return output;
}

double degToRad(double input){
  double output = (input * 3.14159) / 180.0;
  return output;
}
