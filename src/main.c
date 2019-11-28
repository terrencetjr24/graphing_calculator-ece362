// (c) Rados≥aw KwiecieÒ, radek@dxp.pl
// http://en.radzio.dxp.pl
//-------------------------------------------------------------------------------------------------
#include "KS0108.h"
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "string.h"
#include <math.h>
#include "parsing.h"
#include <stdint.h>

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

//void micro_wait(unsigned int); //added to parsing.h

// Start of keypad setup
int col = 0;
int8_t history[16] = {0};
int8_t lookup[16] = {1,4,7,0xe,2,5,8,0,3,6,9,0xf,0xa,0xb,0xc,0xd};
char char_lookup[16] = {'1','4','7','*','2','5','8','0','3','6','9','#','A','B','C','D'};
char screen[2][8][64] = {0};

uint16_t tim2arr;


uint16_t wavetable[80];


void setup_GPIOB(void){
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
  GPIOB->MODER = GPIO_MODER_MODER10_1;
  GPIOB->AFR[1] |= 0x200;
}

void setup_tim2(void){
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  TIM2->ARR = 1750;//17500-1;
  TIM2->PSC = 16;
  TIM2->CR1 &=~ TIM_CR1_DIR;
  TIM2->CR1 &=~ TIM_CR1_CMS;
  TIM2->CR1 |= TIM_CR1_ARPE;
  TIM2->CCMR2 &=~ TIM_CCMR2_CC3S | TIM_CCMR2_OC3M;
  TIM2->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1;
  TIM2->CCER |= TIM_CCER_CC3E;
  TIM2->BDTR |= TIM_BDTR_MOE;
  TIM2->CR1 |= TIM_CR1_ARPE;
  TIM2->CCR3 = 3500/2;
  TIM2->CR1 |= TIM_CR1_CEN;
}

void setup_dma(void) {
    // Your code goes here.
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel3->CCR &= ~DMA_CCR_EN;
    DMA1_Channel3->CMAR = (uint32_t)(wavetable);
    DMA1_Channel3->CPAR = (uint32_t)&(TIM2->ARR);
    DMA1_Channel3->CNDTR = 80;
    DMA1_Channel3->CCR |= DMA_CCR_CIRC;
    DMA1_Channel3->CCR |= DMA_CCR_MSIZE_1 | DMA_CCR_PSIZE_1 | DMA_CCR_MINC | DMA_CCR_DIR;
    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

void init_wavetable(void){
    int x = 0;
    //wavetable[tablesize-1] = 1;
    while(x < 80){
        wavetable[x] = 3500 + 200*sin(2*3.14/80*x);
        x = x + 1;
    }

}

void reset_wavetable(void){
    for(int i = 0; i < 80; i++) wavetable[i] = 1;
}
// Writing pixels to screen is not working in manual function, so using built-in
// library function to do pixel writing instead
/*
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

}

void on(int x, int y) {
    int half = x / 64;
    int row = 7 - y / 8;
    int col = x % 64;

    screen[half][row][col] |= 1 << (7 - (y % 8));

    pixel_to_screen(half, row, col);
}

void off(int x, int y) {
    int half = x / 64;
    int row = 7 - y / 8;
    int col = x % 64;

    screen[half][row][col] &= ~(1 << (7 - (y % 8)));

    pixel_to_screen(half, row, col);
}*/

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
    TIM6->DIER |= TIM_DIER_UDE;
    TIM6->CR2 |= TIM_CR2_MMS_1;
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
int get_key_pressed() {
    int key = get_key_press();
    while(key != get_key_release());
    return key;
}

char get_char_key() {
    int index = get_key_pressed();
    return char_lookup[index];
}


void init_hardware(void)
{
    setup_tim2();
    setup_GPIOB();
    init_wavetable();
    GLCD_Initalize();
    GLCD_ClearScreen();
    init_keypad();
    setup_dma();
    setup_timer6();
    setup_gpio();
}

void graph(void) {

    //GLCD_GoTo(0,0);
    //GLCD_WriteString("it works");
    // Axes
    for(int i = 0; i < 128; i++)
    {
        //on(i,32);
        GLCD_SetPixel(i,32,'b');
    }

    for(int i = 0; i < 64; i++)
    {
        //on(64,i);
        GLCD_SetPixel(64,i,'b');
    }
    //GLCD_GoTo(0,2);
    //GLCD_WriteString("it works 2");

    float maxX = 50;
    float minX = -1 * maxX;
    float xstep = maxX * 2 / 128;
    //float maxY = 100;
    float ystep = 5;
    float curX = 0;
    float curY = 0;
    int flag = 0;
    //char buffer[5];
    for (int xpix = 0; xpix < 128; xpix++)
    {
        curX = xpix * xstep + minX;
        // call f(curX) here
        curY = curX * curX + 20; // use y = f(x) = x^2 for now
        //curY = sin(curX);
        int ypix = (int) curY / ystep + 32;
        //itoa(ypix,buffer,10);   // here 2 means binary
        if (ypix >= 64 || ypix < 0)
        {
            flag = 1;
            //break;
        }
        else
        {
            GLCD_SetPixel(xpix,ypix,'b');
        }
    }
    //GLCD_GoTo(0,1);
    //GLCD_WriteString("it works 2");

}

int main(void)
{
    uint8_t codestack[STACK_SIZE];
    memset(&codestack,-1,sizeof codestack);

    char expression[46] = ">"; // 46 to allow for overflow of string - doesn't actually store in expression
    uint8_t index = 1;
    char result[100] = ">"; // 6 characters
    //char error_mess[STACK_SIZE / 2 + 1] = "YOU IDIOT! Press D";
    //Memory reduction
    uint8_t error = 0;
    uint8_t alternateFunc = NORMAL;

    init_hardware();
/*
    GLCD_GoTo(0,0);
    GLCD_WriteString("testing");
    micro_wait(1000000);
    GLCD_ClearRow(0);
    graph();
    micro_wait(1000000);
    GLCD_GoTo(0,7);
    GLCD_WriteString("testing");
    micro_wait(1000000);
    GLCD_ClearScreen();
    micro_wait(1000000);
*/

    char line1[21];
    memcpy(line1,expression,20);
    line1[20] = '\0';
    char line2[21];
    memcpy(line2,&(expression[20]),20);
    line2[20] = '\0';
    char clear[21] = "                     ";

    GLCD_GoTo(0,0);
    GLCD_WriteString(line1);//write first half of expression
    //GLCD_GoTo(0,3);
    //GLCD_WriteString("Alt Func. Status: NUM");
    GLCD_GoTo(0,4);
    GLCD_WriteString("0-9 = Numbers");
    GLCD_GoTo(0,5);
    GLCD_WriteString("* = '.'  |  # = '-'  ");
    GLCD_GoTo(0,6);
    GLCD_WriteString("A = AF1  |  B = AF2  ");
    GLCD_GoTo(0,7);
    GLCD_WriteString("C = BKSP |  D = ENTER");
    int bkspStack[STACK_SIZE+1] = {0};
    int bkspIndex = 0;

    while (1)
    {
        char key = get_char_key();
        error = stackManipulation(codestack, expression, &index, result, key, &alternateFunc, bkspStack, &bkspIndex);

        memcpy(line1,expression,20);
        line1[20] = '\0';
        memcpy(line2,&(expression[20]),20);
        line2[20] = '\0';

        GLCD_GoTo(0,0);
        GLCD_WriteString(line1);//write first half of expression
        GLCD_GoTo(0,1);
        GLCD_WriteString(line2);//write second half of expression


        if (error == 1)
        {
            //DMA1_Channel3->CCR &= ~DMA_CCR_EN;
            //insert beeping sound here
            GLCD_GoTo(0,2);
            GLCD_WriteString(result);//write output
            GLCD_GoTo(0,3);
            GLCD_WriteString("PRESS ENTER!!!");//write first half of expression
            // beep begins
            //print out the error message
            while (get_char_key() != 'D');

            // clear screen
            //ClearScreenFunction()
            memset(expression,0,sizeof expression);
            strcpy(expression, ">");
            error = 0;
            index = 1;
            strcpy(line1,">                    ");
            strcpy(line2,"                     ");
            GLCD_GoTo(0,0);
            GLCD_WriteString(line1);//write first half of expression
            GLCD_GoTo(0,1);
            GLCD_WriteString(line2);//write second half of expression
            GLCD_GoTo(0,2);
            GLCD_WriteString(line2);
        }
        else if (error == 2)
        {
            tim2arr = 3500;

            //Formatting the answer
            strcpy(line2,"                     ");
            GLCD_GoTo(0,2);
            GLCD_WriteString(line2);//write output
            strcpy(line2, "Ans: ");
            strcat(line2, result);
            GLCD_GoTo(0,2);
            GLCD_WriteString(line2);//write output

            GLCD_GoTo(0,3);
            GLCD_WriteString("                     ");
            GLCD_GoTo(0,4);
            GLCD_WriteString("                     ");
            GLCD_GoTo(0,5);
            GLCD_WriteString("                     ");
            GLCD_GoTo(0,6);
            GLCD_WriteString("                     ");
            GLCD_GoTo(0,7);
            GLCD_WriteString("                     ");

            GLCD_GoTo(0,4);
            GLCD_WriteString("Press D to continue");

            while (get_char_key() != 'D');


            GLCD_GoTo(0,4);
            GLCD_WriteString("                     ");
            GLCD_GoTo(0,5);
            GLCD_WriteString("                     ");
            GLCD_GoTo(0,6);
            GLCD_WriteString("                     ");
            GLCD_GoTo(0,7);
            GLCD_WriteString("                     ");
            //Formatting the previous answer
            GLCD_GoTo(0,2);
            GLCD_WriteString("Prev Ans: ");//write output
            GLCD_GoTo(0,3);
            GLCD_WriteString(result);//write output
            // clear screen
            //ClearScreenFunction()
            memset(expression,0,sizeof expression);
            strcpy(expression, ">");
            error = 0;
            index = 1;
            strcpy(line1,">                    ");
            strcpy(line2,"                     ");
            GLCD_GoTo(0,0);
            GLCD_WriteString(line1);//write first half of expression
            GLCD_GoTo(0,1);
            GLCD_WriteString(line2);//write second half of expression
        }
        else if (error == 3){
            //BACKSPACE FUNCTION
            if(index > 1) {
                index-=bkspStack[bkspIndex-1];
                memcpy(&(expression[index]),"      ",bkspStack[--bkspIndex]);
            }
            memcpy(line1,expression,20);
            line1[20] = '\0';
            memcpy(line2,&(expression[20]),20);
            line2[20] = '\0';
            GLCD_GoTo(0,0);
            GLCD_WriteString(line1);//write first half of expression
            GLCD_GoTo(0,1);
            GLCD_WriteString(line2);//write second half of expression
        }

        GLCD_GoTo(0,3);
        GLCD_WriteString("                     ");
        GLCD_GoTo(0,4);
        GLCD_WriteString("                     ");
        GLCD_GoTo(0,5);
        GLCD_WriteString("                     ");
        GLCD_GoTo(0,6);
        GLCD_WriteString("                     ");
        GLCD_GoTo(0,7);
        GLCD_WriteString("                     ");

        if(alternateFunc == NORMAL){
            GLCD_GoTo(0,4);
            GLCD_WriteString("0-9 = Numbers");
            GLCD_GoTo(0,5);
            GLCD_WriteString("* = '.'  |  # = '-'  ");
            GLCD_GoTo(0,6);
            GLCD_WriteString("A = AF1  |  B = AF2  ");
            GLCD_GoTo(0,7);
            GLCD_WriteString("C = BKSP |  D = ENTER");
        }
        else if(alternateFunc == ALTERNATE_1){
            GLCD_GoTo(0,4);
            GLCD_WriteString("1='+' | 2='-' | 3='/'");
            GLCD_GoTo(0,5);
            GLCD_WriteString("4='*' | 5='(' | 6=')'");
            GLCD_GoTo(0,6);
            GLCD_WriteString("7= e^ | 8=ln( | 9=log");
            GLCD_GoTo(0,7);
            GLCD_WriteString("*='^' | 0=CLR | #=ANS");

        }
        else{
            GLCD_GoTo(0,4);
            GLCD_WriteString("1=sin | 2=cos |3=tan");
            GLCD_GoTo(0,5);
            GLCD_WriteString("4=asin| 5=acos|6=atan");
            GLCD_GoTo(0,6);
            GLCD_WriteString("7= !( | 8='X' |9= PI");
            GLCD_GoTo(0,7);
            GLCD_WriteString("*=GRAPH|0=DtoR|#=RtoD");
        }

    }
    return 0;
}

