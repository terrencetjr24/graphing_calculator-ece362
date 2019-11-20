// (c) Rados≥aw KwiecieÒ, radek@dxp.pl
// http://en.radzio.dxp.pl
//-------------------------------------------------------------------------------------------------
#include "KS0108.h"
#include "graphic.h"
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "string.h"
#include "parsing.h"
#include <math.h>

#define GPIO_ODR_D0                 ((uint32_t)0x00000001)
#define GPIO_ODR_D1                 ((uint32_t)0x00000002)
#define GPIO_ODR_D2                 ((uint32_t)0x00000004)
#define GPIO_ODR_D3                 ((uint32_t)0x00000008)
#define GPIO_ODR_D4                 ((uint32_t)0x00000010)
#define GPIO_ODR_D5                 ((uint32_t)0x00000020)
#define GPIO_ODR_D6                 ((uint32_t)0x00000040)
#define GPIO_ODR_D7                 ((uint32_t)0x00000080)
#define GPIO_ODR_CS1                ((uint32_t)0x00000100)
#define GPIO_ODR_CS2                ((uint32_t)0x00000200)
#define GPIO_ODR_RW                 ((uint32_t)0x00000400)
#define GPIO_ODR_RS                 ((uint32_t)0x00000800)
#define GPIO_ODR_EN                 ((uint32_t)0x00001000)

void micro_wait(unsigned int);

// Start of keypad setup
int col = 0;
int8_t history[16] = {0};
int8_t lookup[16] = {1,4,7,0xe,2,5,8,0,3,6,9,0xf,0xa,0xb,0xc,0xd};
char char_lookup[16] = {'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'};
char screen[2][8][64] = {0};
//char screen[2][64][8] = {0};

void on(int x, int y) {
    int half = x / 64;
    int row = 7 - y / 8;
    //int row = y;
    int col = x % 64;
    //int col = x / 8;

    /*int c = col;
    char snum[10] = {0};
    itoa(c, snum, 10);
    GLCD_GoTo(0,0);
    GLCD_WriteString(snum);*/
    screen[half][row][col] |= 1 << (7 - (y % 8));

    pixel_to_screen(half, row, col);
}

void off(int x, int y) {
    int half = x / 64;
    int row = y /8;
    int col = x % 64;

    screen[half][row][col] &= ~(1 << (y % 8));

    pixel_to_screen(half, row, col);
}

void pixel_to_screen(int half, int row, int col) {

    GPIOA->ODR &= ~0xffff;

    GPIOA->ODR &= ~GPIO_ODR_RW;
    GPIOA->ODR &= ~GPIO_ODR_RS;

    GPIOA->ODR &= ~GPIO_ODR_CS1;
    GPIOA->ODR &= ~GPIO_ODR_CS2;

    if(half == 1)
    {
        GPIOA->ODR |= GPIO_ODR_CS1;
    }
    else
    {
        GPIOA->ODR |= GPIO_ODR_CS2;
    }

    GPIOA->ODR |= GPIO_ODR_EN;
    micro_wait(100);
    GPIOA->ODR &= ~GPIO_ODR_EN;
    micro_wait(100);

    GPIOA->ODR &= ~0xff;

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
    GPIOA->ODR |= GPIO_ODR_EN;
    micro_wait(100);
    GPIOA->ODR &= ~GPIO_ODR_EN;
    micro_wait(100);

    GPIOA->ODR &= ~0xff;

    GPIOA->ODR |= col | 0x40; //setting x address
    GPIOA->ODR |= GPIO_ODR_EN;
    micro_wait(100);
    GPIOA->ODR &= ~GPIO_ODR_EN;
    micro_wait(100);

    GPIOA->ODR &= ~0xff;

    GPIOA->ODR |= GPIO_ODR_RS;
    GPIOA->ODR |= screen[half][row][col];
    GPIOA->ODR |= GPIO_ODR_EN;
    micro_wait(100);
    GPIOA->ODR &= ~GPIO_ODR_EN;
    micro_wait(100);

    GPIOA->ODR &= ~0xff;
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
    //on(0,0);
    /*for(int i = 0; i < 128; i++) {
        on(i,32);
    }

    /*for(int i = 0; i < 64; i++)
    {
        on(64,i);
    }

    for(int i = 0; i < 128; i++)
    {
        on(i,i / 2);
    }*/

    //int h = 0;
    //int r = 1;
    //int c = 10;
    //screen[0][7][0] = 0x80;
    //pixel_to_screen(0, 7, 0);

    //on(30,32);
    //char test[1] = {screen[h][r][c]};
    //GLCD_GoTo(0,0);
    //GLCD_WriteString(test);

    int lineLength = 20;
    char expression[42] = {'>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
    char line1[21] = {'>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
    char line2[21] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};

    int index = 1;
    int af1 = 0;
    int af2 = 0;

    GLCD_GoTo(0,0);
    GLCD_WriteString(line1);

    /*char snum[21] = {'0','0','0'};
    GLCD_GoTo(0,6);
    GLCD_WriteString(snum);

    snum[3] = '1';
    GLCD_GoTo(0,7);
    GLCD_WriteString(snum);*/

    while (1)
    {
        char key = get_char_key();

        if (key == 'C')
        {
            index = 0;
            for (int i = 0; i < lineLength * 2; i++)
            {
                expression[i] = ' ';
            }

            expression[0] = '>';
            /*for (int i = 0; i < lineLength; i++)
            {
                line1[i] = ' ';
                line2[i] = ' ';
            }
            line1[0] = '>';
            GLCD_GoTo(0,3);
            GLCD_WriteString(line1);*/

        }

        if (key == 'D')
        {
            break;
        }

        if(af1 == 1) {
            af1 = 0;
            switch(key)
            {
                case 'A':
                    af1 = 0;
                    index--;
                    break;
                case 'B':
                    af2 = 1;
                    af1 = 0;
                    index--;
                    break;
                case 'C':
                    break;
                case '0':
                    expression[index] = '^';
                    break;
                case '1':
                    expression[index] = '+';
                    break;
                case '2':
                    expression[index] = '-';
                    break;
                case '3':
                    expression[index] = '/';
                    break;
                case '4':
                    expression[index] = '*';
                    break;
                case '5':
                    expression[index] = '^';
                    break;
                case '6':
                    expression[index] = '^';
                    break;
                case '7':
                    expression[index] = 'e';
                    index++;
                    expression[index] = '^';
                    break;
                case '8':
                    expression[index] = 'l';
                    index++;
                    expression[index] = 'n';
                    index++;
                    expression[index] = '(';
                    break;
                case '9':
                    expression[index] = 'l';
                    index++;
                    expression[index] = 'o';
                    index++;
                    expression[index] = 'g';
                    index++;
                    expression[index] = '(';
                    break;
                case '*':
                    expression[index] = '(';
                    break;
                case '#':
                    expression[index] = ')';
                    break;
                default:
                    break;
            }
        }
        else if(af2 == 1) {
            af2 = 0;
            switch(key)
            {
                case 'A':
                    af1 = 1;
                    af2 = 0;
                    index--;
                    break;
                case 'B':
                    af2 = 0;
                    index--;
                    break;
                case 'C':
                    break;
                case '0':
                    expression[index] = 'G';
                    break;
                case '1':
                    GLCD_GoTo(0,3);
                    //GLCD_WriteString("This is case 1");
                    expression[index] = 's';
                    index++;
                    expression[index] = 'i';
                    index++;
                    expression[index] = 'n';
                    index++;
                    expression[index] = '(';
                    break;
                case '2':
                    expression[index] = 'c';
                    index++;
                    expression[index] = 'o';
                    index++;
                    expression[index] = 's';
                    index++;
                    expression[index] = '(';
                    break;
                case '3':
                    expression[index] = 't';
                    index++;
                    expression[index] = 'a';
                    index++;
                    expression[index] = 'n';
                    index++;
                    expression[index] = '(';
                    break;
                case '4':
                    expression[index] = 'a';
                    index++;
                    expression[index] = 's';
                    index++;
                    expression[index] = 'i';
                    index++;
                    expression[index] = 'n';
                    index++;
                    expression[index] = '(';
                    break;
                case '5':
                    expression[index] = 'a';
                    index++;
                    expression[index] = 'c';
                    index++;
                    expression[index] = 'o';
                    index++;
                    expression[index] = 's';
                    index++;
                    expression[index] = '(';
                    break;
                case '6':
                    expression[index] = 'a';
                    index++;
                    expression[index] = 't';
                    index++;
                    expression[index] = 'a';
                    index++;
                    expression[index] = 'n';
                    index++;
                    expression[index] = '(';
                    break;
                case '7':
                    expression[index] = '!';
                    break;
                case '8':
                    expression[index] = 'x';
                    break;
                case '9':
                    expression[index] = 'x';
                    break;
                case '*':
                    expression[index] = 'd';
                    index++;
                    expression[index] = 't';
                    index++;
                    expression[index] = 'o';
                    index++;
                    expression[index] = 'r';
                    index++;
                    expression[index] = '(';
                    break;
                case '#':
                    expression[index] = 'r';
                    index++;
                    expression[index] = 't';
                    index++;
                    expression[index] = 'o';
                    index++;
                    expression[index] = 'd';
                    index++;
                    expression[index] = '(';
                    break;
                default:
                    break;
            }
        }
        else {
            switch(key)
            {
                case 'A':
                    af1 = 1;
                    index--;
                    break;
                case 'B':
                    af2 = 1;
                    index--;
                    break;
                case 'C':
                    break;
                case '*':
                    expression[index] = '.';
                    break;
                case '#':
                    expression[index] = '-';
                    break;
                default:
                    expression[index] = key;
                    break;

            }
        }

        index++;

        /*int countup1 = 0;
        int countup2 = 0;

        if (index < lineLength)
        {
            countup1 = index;
            countup2 = 0;
        }
        else
        {
            countup1 = lineLength;
            countup2 = index - lineLength;
        }

        for (int i = 0; i < countup1; i++)
        {
            line1[i] = expression[i];
        }

        for (int i = 0; i < countup2; i++)
        {
            line2[i] = expression[i + lineLength];
        }*/

        for (int i = 0; i < lineLength; i++)
        {
            line1[i] = expression[i];
            line2[i] = expression[i + lineLength];
        }

        GLCD_GoTo(0,0);
        GLCD_WriteString(line1);
        GLCD_GoTo(0,1);
        GLCD_WriteString(line2);

        //index++;
        micro_wait(1000);
    }

    GLCD_GoTo(0,0);
    GLCD_WriteString("Are you sure?");
    micro_wait(2000000);
    GLCD_GoTo(0,1);
    GLCD_WriteString("Press 1 if you dare");

    char key1 = get_char_key();

    if (key1 == '1')
    {
        GLCD_GoTo(0,1);
        GLCD_WriteString("                   ");
        GLCD_GoTo(0,0);
        GLCD_WriteString("You know the answer");
        micro_wait(2000000);
        GLCD_GoTo(0,1);
        GLCD_WriteString("I know you do");
        micro_wait(3000000);
        GLCD_GoTo(0,2);
        GLCD_WriteString(":)");
    }
    else
    {
        GLCD_GoTo(0,0);
        GLCD_WriteString("                   ");
        GLCD_GoTo(0,1);
        GLCD_WriteString("                   ");
        GLCD_GoTo(0,0);
        GLCD_WriteString("You chicken");
    }


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
