//The beginning of the process to parsing

//The purpose of this file is to make sure that we can output the value from floating point operations
//and that this vlaue is correct

#include <math.h>
#include <stdlib.h>

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include "fifo.h"

#define UNK -1
#define NON_INTR 0
#define INTR 1

int __io_putchar(int ch);
static int putchar_nonirq(int ch);

void prob3(void);
void prob4(void);
void prob5(void);

static struct fifo input_fifo;  // input buffer
static struct fifo output_fifo; // output buffer
int interrupt_mode = UNK;   // which version of putchar/getchar to use.
int echo_mode = 1;          // should we echo input characters?

#define RATE 100000
#define N 1000
#define M_PI 3.14159
short int wavetable[N];

//=============================================================================
// This is a version of printf() that will disable interrupts for the
// USART and write characters directly.  It is intended to handle fatal
// exceptional conditions.
// It's also an example of how to create a variadic function.
static void safe_printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    char buf[80];
    int len = vsnprintf(buf, sizeof buf, format, ap);
    int saved_txeie = USART1->CR1 & USART_CR1_TXEIE;
    USART1->CR1 &= ~USART_CR1_TXEIE;
    int x;
    for(x=0; x<len; x++) {
        putchar_nonirq(buf[x]);
    }
    USART1->CR1 |= saved_txeie;
    va_end(ap);
}

//=======================================================================
// Simply write a string one char at a time.
//=======================================================================
static void putstr(const char *s) {
    while(*s)
        __io_putchar(*s++);
}

//=======================================================================
// Insert a character and echo it.
// (or, if it's a backspace, remove a char and erase it from the line).
// If echo_mode is turned off, just insert the character and get out.
//=======================================================================
static void insert_echo_char(char ch) {
    if (ch == '\r')
        ch = '\n';
    if (!echo_mode) {
        fifo_insert(&input_fifo, ch);
        return;
    }
    if (ch == '\b' || ch == '\177') {
        if (!fifo_empty(&input_fifo)) {
            char tmp = fifo_uninsert(&input_fifo);
            if (tmp == '\n')
                fifo_insert(&input_fifo, '\n');
            else if (tmp < 32)
                putstr("\b\b  \b\b");
            else
                putstr("\b \b");
        }
        return; // Don't put a backspace into buffer.
    } else if (ch == '\n') {
        __io_putchar('\n');
    } else if (ch == 0){
        putstr("^0");
    } else if (ch == 28) {
        putstr("^\\");
    } else if (ch < 32) {
        __io_putchar('^');
        __io_putchar('A'-1+ch);
    } else {
        __io_putchar(ch);
    }
    fifo_insert(&input_fifo, ch);
}


//-----------------------------------------------------------------------------
// Section 6.2
//-----------------------------------------------------------------------------
// This should should perform the following
// 1) Enable clock to GPIO port A
// 2) Configure PA9 and PA10 to alternate function to use a USART
//    Note: Configure both MODER and AFRL registers
// 3) Enable clock to the USART module, it is up to you to determine
//    which RCC register to use
// 4) Disable the USART module (hint UE bit in CR1)
// 5) Configure USART for 8 bits, 1 stop bit and no parity bit
// 6) Use 16x oversampling
// 7) Configure for 115200 baud rate
// 8) Enable the USART for both transmit and receive
// 9) Enable the USART
// 10) Wait for TEACK and REACK to be set by hardware in the ISR register
// 11) Set the 'interrupt_mode' variable to NON_INTR
void tty_init(void) {
    // Disable buffers for stdio streams.  Otherwise, the first use of
    // each stream will result in a *malloc* of 2K.  Not good.
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    // Student code goes here...
    //Turn on GPIOA clock
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    //Enable clock to USART
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    //Clearing pin 9 and 10, setting them to Alt. Func.
    GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
    GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);
    //Clearing then Setting alt. Funcs. of pins 9 and 10 in AFRH (not AFRL)
    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH1 | GPIO_AFRH_AFRH2);
    GPIOA->AFR[1] |= (0x00000110); //1<<(4*1) | 1<<(4*2);
    //Turning off the USART
    USART1->CR1 &= ~USART_CR1_UE;
    //Configuring for 8 bit mode, w/ 1 stop bit and no parity bit (clear M1, M0, and PCE), and setting to 16x sampling (clearing OVER8)
    USART1->CR1 &= ~(USART_CR1_M | (USART_CR1_M<<16) | USART_CR1_PCE | USART_CR1_OVER8);
    //Clearing STOP (held in CR2)
    USART1->CR2 &= ~USART_CR2_STOP;
    //Configuring for a 115,200 baud rate (48,000,000 / 417 = 115,200)
    USART1->BRR |= 0x1a1;
    //Enable USART to receive and transmit, and enabling the UART
    USART1->CR1 |= (USART_CR1_RE | USART_CR1_TE | USART_CR1_UE);
    //Waiting for TEACH AND REACK to be set
    while( (USART1->ISR & (USART_ISR_REACK | USART_ISR_TEACK)) == 0 );
    //Setting interrupt_mode in NON_INTR
    interrupt_mode = NON_INTR;
}

//=======================================================================
// Enable the USART RXNE interrupt.
// Remember to enable the right bit in the NVIC registers
//=======================================================================
void enable_tty_irq(void) {
    // Student code goes here...
    //Enabling RXNE interrupt
    USART1->CR1 |= USART_CR1_RXNEIE;
    //Enabling ?correct? bit of the NVIC register
    NVIC->ISER[0] |= 1<<USART1_IRQn;
    //Setting interrupt_mode to INTR
    interrupt_mode = INTR;
}

//-----------------------------------------------------------------------------
// Section 6.3
//-----------------------------------------------------------------------------
//=======================================================================
// This method should perform the following
// Transmit 'ch' using USART1, remember to wait for transmission register to be
// empty. Also this function must check if 'ch' is a new line character, if so
// it must transmit a carriage return before transmitting 'ch' using USART1.
// Think about this, why must we add a carriage return, what happens otherwise?
//=======================================================================
static int putchar_nonirq(int ch) {
    // Student code goes here...
    if(ch == 10){ //10 == ASCII '\n'
        while((USART1->ISR & USART_ISR_TXE) == 0 );
        USART1->TDR = 13; //13 == ASCII '\r'
    }
    while((USART1->ISR & USART_ISR_TXE) == 0 );
    USART1->TDR = ch;

    return ch;

}

//-----------------------------------------------------------------------------
// Section 6.4
//-----------------------------------------------------------------------------
// See lab document for description
static int getchar_nonirq(void) {
    //Checking if the overrun flag is set
    if((USART1->ISR & USART_ISR_ORE))
        USART1->ICR &= ~USART_ICR_ORECF;

    while(!fifo_newline(&input_fifo)){
        while((USART1->ISR & USART_ISR_RXNE) == 0);
        insert_echo_char(USART1->RDR);
    }

    return fifo_remove(&input_fifo);
}

//-----------------------------------------------------------------------------
// Section 6.5
//-----------------------------------------------------------------------------
// See lab document for description
//=======================================================================
// IRQ invoked for USART1 activity.
void USART1_IRQHandler(void) {
    // Student code goes here...
    if((USART1->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
        insert_echo_char(USART1->RDR);
    if((USART1->ISR & USART_ISR_TXE) == USART_ISR_TXE){
        if(fifo_empty(&output_fifo))
            USART1->CR1 &= ~USART_CR1_TXEIE;
        else
            USART1->TDR= fifo_remove(&output_fifo);
    }
    //-----------------------------------------------------------------
    // Leave this checking code here to make sure nothing bad happens.
    if (USART1->ISR & (USART_ISR_RXNE|
            USART_ISR_ORE|USART_ISR_NE|USART_ISR_FE|USART_ISR_PE)) {
        safe_printf("Problem in USART1_IRQHandler: ISR = 0x%x\n", USART1->ISR);
    }
}

// See lab document for description
static int putchar_irq(char ch) {
    while(fifo_full(&output_fifo))
        asm("wfi");

    if(ch == '\n')
        fifo_insert(&output_fifo, '\r');
    else
        fifo_insert(&output_fifo, ch);

    if((USART1->ISR & USART_ISR_TXE) == USART_ISR_TXE){
        USART1->CR1 |= USART_CR1_TXEIE;
        USART1_IRQHandler();
    }
    if(ch == '\n'){
        while(fifo_full(&output_fifo))
            asm("wfi");
        fifo_insert(&output_fifo, '\n');
    }
    return ch;
}
// See lab document for description
//This is right if the lab manual is correct
static int getchar_irq(void) {
    while(!fifo_newline(&input_fifo))
        asm("wfi");
    return fifo_remove(&input_fifo);
}

//=======================================================================
// Called by the Standard Peripheral library for a write()
int __io_putchar(int ch) {
    if (interrupt_mode == INTR)
        return putchar_irq(ch);
    else
        return putchar_nonirq(ch);
}

//=======================================================================
// Called by the Standard Peripheral library for a read()
int __io_getchar(void) {
    // Choose the right implementation.
    if (interrupt_mode == INTR)
        return getchar_irq();
    else
        return getchar_nonirq();
}

//-----------------------------------------------------------------------------
// Section 6.6
//-----------------------------------------------------------------------------
//===========================================================================
// This function
// 1) enables clock to port A,
// 2) sets PA4 to analog mode
void setup_gpio() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    GPIOA->MODER |= GPIO_MODER_MODER4;
}

// This function should enable the clock to the
// onboard DAC, enable trigger,
// setup software trigger and finally enable the DAC.
void setup_dac() {
    //Enable clock
        RCC->APB1ENR |= RCC_APB1ENR_DACEN;
        /* These were included in the demo slides but if the channel is already disabled and the buffer already on, they're unnecessary
         * DAC->CR &= ~DAC_CR_EN1;
         * DAC->CR &= ~DAC_CR_BOFF1;
         */
    //Enable the trigger
        DAC->CR |= DAC_CR_TEN1;
    //Setup software trigger
        DAC->CR |= DAC_CR_TSEL1;
    //Enable the DAC
        DAC->CR |= DAC_CR_EN1;
}

// This function should,
// enable clock to timer6,
// setup pre scalar and arr so that the interrupt is triggered every
// 10us, enable the timer 6 interrupt, and start the timer.
void setup_timer6() {
    //Enabling clock of TIM6
        RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    //Assumption: 1 interrupt every 10us = 0.000 01s (480 / 48,000,000 = 0.000010)
    //Setting pre scalar to
        TIM6->PSC = 23;     //(24-1)
    //Setting arr to
        TIM6->ARR = 19;     //(20-1)
    //Enabling TIM6 interrupt and starting the timer
        TIM6->DIER |= TIM_DIER_UIE;
        TIM6->CR1 |= TIM_CR1_CEN;
        NVIC->ISER[0] |= 1<<TIM6_DAC_IRQn;
}


int offset = 0;
int changingValue = 698;
int step = 698.0 * N / 100000.0 * (1 << 16);
void TIM6_DAC_IRQHandler() {
    //Waiting for the DAC to be ready for another conversion
        while((DAC->SWTRIGR & DAC_SWTRIGR_SWTRIG1) == DAC_SWTRIGR_SWTRIG1);
        //DAC->DHR12R1 = ; this step was already done in dac interrupt
    //Initiating the conversion (asserting the software trigger)
        DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
    //Acknowledging the interrupt
        //EXTI->PR ^= (1 << TIM6_DAC_IRQn);
        TIM6->SR &= ~TIM_SR_UIF;
    //for the DAC portion
        offset += step;
        if ((offset>>16) >= N) // If we go past the end of the array,
            offset -= N<<16; // wrap around with same offset as overshoot.
        int sample = wavetable[offset>>16]; // get a sample
        sample = sample / 16 + 2048; // adjust for the DAC range
        DAC->DHR12R1 = sample;
/*
    //Combining a second signal in DAC portion
        offset2 += step2;
            if ((offset2>>16) >= N) // If we go past the end of the array,
                offset2 -= N<<16; // wrap around with same offset as overshoot.
            int sample2 = wavetable[offset2>>16]; // get a sample
            sample2 = sample2 / 16 + 2048; // adjust for the DAC range
            //Directly adding the two samples together
            DAC->DHR12R1 += sample2;
    //If I'd like to combine them and then make sure they're within range, last 2 steps

            sample += sample2;
            sample = sample / 16 + 2048; // adjust for the DAC range
            DAC->DHR12R1 = sample;
            */

}

void init_wavetable(void)
{
    int x;
    for(x=0; x<N; x++)
        wavetable[x] = 32767 * sin(2 * M_PI * x / N);
}

// Act on a command read by testbench().
static void action(char **words) {
    if (words[0] != 0) {
        if (strcasecmp(words[0],"alpha") == 0) {
            // Print the alphabet repeatedly until you press <Enter>.
            char buf[81];
            for(int x=0; x<80; x++)
                buf[x] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[x%26];
            buf[80] = '\0';
            echo_mode = 0;
            for(;;) {
                putstr(buf);
                if (fifo_newline(&input_fifo)) {
                    echo_mode = 1;
                    return;
                }
            }
        }

        if (strcasecmp(words[0],"gen") == 0) {
            if (strcasecmp(words[1],"0") != 0) {
                changingValue = atoi(words[1]);
                step = changingValue * 1000.0 / 100000.0 * (1<<16);
                printf("The new freq: %d\n", changingValue); //Can also delete this
                printf("freq reconfigured\n");
                return;
            }
        }

        if (strcasecmp(words[0],"init") == 0) {
            if (strcasecmp(words[1],"dac") == 0) {
                setup_gpio();
                setup_dac();
                setup_timer6();
                init_wavetable();
                printf("dac initialized\n"); //Can delete this later
                return;
            }

            if (strcasecmp(words[1],"lcd") == 0) {
                printf("lcd command not implemenented yet\n");
                return;
            }
        }
        if (strcasecmp(words[0],"display1") == 0) {
            printf("display1 command not implemented yet\n");
            return;
        }
        if (strcasecmp(words[0],"display2") == 0) {
            printf("display2 command not implemented yet\n");
            return;
        }
        if (strcasecmp(words[0],"display2") == 0) {
            printf("display2 command not implemented yet\n");
            return;
        }

        printf("Unrecognized command: %s\n", words[0]);
    }
}

//===========================================================================
// Interact with the hardware.
// This subroutine waits for a line of input, breaks it apart into an
// array of words, and passes that array of words to the action()
// subroutine.
// The "display1" and "display2" are special words that tell it to
// keep everything after the first space together into words[1].
//
void testbench(void) {
    printf("STM32 testbench.\n");
    for(;;) {
        char buf[60];
        printf("> ");
        fgets(buf, sizeof buf - 1, stdin);
        int sz = strlen(buf);
        if (sz > 0)
            buf[sz-1] = '\0';
        char *words[7] = { 0,0,0,0,0,0,0 };
        int i;
        char *cp = buf;
        for(i=0; i<6; i++) {
            // strtok tokenizes a string, splitting it up into words that
            // are divided by any characters in the second argument.
            words[i] = strtok(cp," \t");
            // Once strtok() is initialized with the buffer,
            // subsequent calls should be made with NULL.
            cp = 0;
            if (words[i] == 0)
                break;
            if (i==0 && strcasecmp(words[0], "display1") == 0) {
                words[1] = strtok(cp, ""); // words[1] is rest of string
                break;
            }
            if (i==0 && strcasecmp(words[0], "display2") == 0) {
                words[1] = strtok(cp, ""); // words[1] is rest of string
                break;
            }
        }
        action(words);
    }
}
static void projAction(char **words) {
    //Converting numbers from arrays of characters to floats
    float arg1 = atof(words[0]);
    float arg2 = atof(words[2]);
    float dummy;
    //establishing how many decimal places being used
    float decimalPlaces = 100000;
    //Initializing my first arguments value (just for printing, don't need if not printing)
    double intPartArg1;
    float decPartArg1;
    int arg1ZeroPadding = -1;
    decPartArg1 = fabs(decimalPlaces * (modf(arg1, &intPartArg1)));
    dummy = decPartArg1;
    while((dummy < decimalPlaces) && (decPartArg1 !=0)){
        dummy *=10;
        arg1ZeroPadding++;
    }
    //Initializing my second arguments value (just for printing, don't need if not printing)
    double intPartArg2;
    float decPartArg2;
    int arg2ZeroPadding = -1;
    decPartArg2 = fabs(decimalPlaces * (modf(arg2, &intPartArg2)));
    dummy = decPartArg2;
    while((dummy < decimalPlaces) && (decPartArg2 !=0)){
        dummy *=10;
        arg2ZeroPadding++;
    }
    //Declaring output values, will initialize in the if statement (ALWAYS NEED THESE)
    double output;
    double intPartOutput;
    int outputZeroPadding = -1;
    float decPartOutput;

    //These are just test cases and they will be handled by the parser later, but the process for printing
    //the output to the screen will stay consistent
/*
    if(strcasecmp(words[1], "+") == 0){
        //Calculating the output
        output = arg1 + arg2;
        decPartOutput = fabs(decimalPlaces * (modf(output, &intPartOutput)));
        if(decPartOutput == decimalPlaces){
                    decPartOutput = 0;
                    intPartOutput += 1;
                }
        dummy = decPartOutput;
        while((dummy < decimalPlaces) && (decPartOutput !=0)){
            dummy *=10;
            outputZeroPadding++;
        }

        if((arg1 < 0) && (arg1 > -1)){
            printf("-%d.",(int)intPartArg1);
            while(arg1ZeroPadding-- > 0)
                printf("0");
            printf("%d", (int)decPartArg1);
        }
        else{
            printf("%d.",(int)intPartArg1);
            while((arg1ZeroPadding--) && (decPartArg1 !=0))
                printf("0");
            printf("%d", (int)decPartArg1);
        }

        printf(" + ");

        if((arg2 < 0) && (arg2 > -1)){
            printf("-%d.",(int)intPartArg2);
            while(arg2ZeroPadding-- > 0)
                printf("0");
            printf("%d", (int)decPartArg2);
        }
        else{
            printf("%d.",(int)intPartArg2);
            while((arg2ZeroPadding--) && (decPartArg2 !=0))
                printf("0");
            printf("%d", (int)decPartArg2);
        }

        printf(" = ");
        if(((int)intPartOutput == 0) && ((int)decPartOutput == 0)){
            printf("0.0");
        }
        else if((output < 0) && (output > -1)){
            printf("-%d.",(int)intPartOutput);
            while(outputZeroPadding-- > 0)
                printf("0");
            printf("%d", (int)decPartOutput);
        }
        else{
            printf("%d.",(int)intPartOutput);
            while((outputZeroPadding--) && (decPartOutput !=0))
                printf("0");
            printf("%d", (int)decPartOutput);
        }

        printf("\n");
    }
    else if(strcasecmp(words[1], "-") == 0){
            output = arg1 - arg2;
            decPartOutput = fabs(decimalPlaces * (modf(output, &intPartOutput)));
            if(decPartOutput == decimalPlaces){
                    decPartOutput = 0;
                    intPartOutput += 1;
                }
            dummy = decPartOutput;
            while((dummy < decimalPlaces) && (decPartOutput !=0)){
                dummy *=10;
                outputZeroPadding++;
            }

            if((arg1 < 0) && (arg1 > -1)){
                printf("-%d.",(int)intPartArg1);
                while(arg1ZeroPadding-- > 0)
                    printf("0");
                printf("%d", (int)decPartArg1);
            }
            else{
                printf("%d.",(int)intPartArg1);
                while((arg1ZeroPadding--) && (decPartArg1 !=0))
                    printf("0");
                printf("%d", (int)decPartArg1);
            }

            printf(" - ");

            if((arg2 < 0) && (arg2 > -1)){
                printf("-%d.",(int)intPartArg2);
                while(arg2ZeroPadding-- > 0)
                    printf("0");
                printf("%d", (int)decPartArg2);
            }
            else{
                printf("%d.",(int)intPartArg2);
                while((arg2ZeroPadding--) && (decPartArg2 !=0))
                    printf("0");
                printf("%d", (int)decPartArg2);
            }

            printf(" = ");
            if(((int)intPartOutput == 0) && ((int)decPartOutput == 0)){
                printf("0.0");
            }
            else if((output < 0) && (output > -1)){
                printf("-%d.",(int)intPartOutput);
                while(outputZeroPadding-- > 0)
                    printf("0");
                printf("%d", (int)decPartOutput);
            }
            else{
                printf("%d.",(int)intPartOutput);
                while((outputZeroPadding--) && (decPartOutput !=0))
                    printf("0");
                printf("%d", (int)decPartOutput);
            }

            printf("\n");
    }
    else if(strcasecmp(words[1], "/") == 0){
            output = arg1 / arg2;
            decPartOutput = fabs(decimalPlaces * (modf(output, &intPartOutput)));
            if(decPartOutput == decimalPlaces){
                    decPartOutput = 0;
                    intPartOutput += 1;
                }
            dummy = decPartOutput;
            while((dummy < decimalPlaces) && (decPartOutput !=0)){
                dummy *=10;
                outputZeroPadding++;
            }

            if((arg1 < 0) && (arg1 > -1)){
                printf("-%d.",(int)intPartArg1);
                while(arg1ZeroPadding-- > 0)
                    printf("0");
                printf("%d", (int)decPartArg1);
            }
            else{
                printf("%d.",(int)intPartArg1);
                while((arg1ZeroPadding--) && (decPartArg1 !=0))
                    printf("0");
                printf("%d", (int)decPartArg1);
            }

            printf(" / ");

            if((arg2 < 0) && (arg2 > -1)){
                printf("-%d.",(int)intPartArg2);
                while(arg2ZeroPadding-- > 0)
                    printf("0");
                printf("%d", (int)decPartArg2);
            }
            else{
                printf("%d.",(int)intPartArg2);
                while((arg2ZeroPadding--) && (decPartArg2 !=0))
                    printf("0");
                printf("%d", (int)decPartArg2);
            }

            printf(" = ");
            if(((int)intPartOutput == 0) && ((int)decPartOutput == 0)){
                printf("0.0");
            }
            else if((output < 0) && (output > -1)){
                printf("-%d.",(int)intPartOutput);
                while(outputZeroPadding-- > 0)
                    printf("0");
                printf("%d", (int)decPartOutput);
            }
            else{
                printf("%d.",(int)intPartOutput);
                while((outputZeroPadding--) && (decPartOutput !=0))
                    printf("0");
                printf("%d", (int)decPartOutput);
            }

            printf("\n");
    }
    else if(strcasecmp(words[1], "*") == 0){
            output = arg1 * arg2;
            decPartOutput = fabs(decimalPlaces * (modf(output, &intPartOutput)));
            if(decPartOutput == decimalPlaces){
                    decPartOutput = 0;
                    intPartOutput += 1;
                }
            dummy = decPartOutput;
            while((dummy < decimalPlaces) && (decPartOutput !=0)){
                dummy *=10;
                outputZeroPadding++;
            }

            if((arg1 < 0) && (arg1 > -1)){
                printf("-%d.",(int)intPartArg1);
                while(arg1ZeroPadding-- > 0)
                    printf("0");
                printf("%d", (int)decPartArg1);
            }
            else{
                printf("%d.",(int)intPartArg1);
                while((arg1ZeroPadding--) && (decPartArg1 !=0))
                    printf("0");
                printf("%d", (int)decPartArg1);
            }

            printf(" * ");

            if((arg2 < 0) && (arg2 > -1)){
                printf("-%d.",(int)intPartArg2);
                while(arg2ZeroPadding-- > 0)
                    printf("0");
                printf("%d", (int)decPartArg2);
            }
            else{
                printf("%d.",(int)intPartArg2);
                while((arg2ZeroPadding--) && (decPartArg2 !=0))
                    printf("0");
                printf("%d", (int)decPartArg2);
            }

            printf(" = ");
            if(((int)intPartOutput == 0) && ((int)decPartOutput == 0)){
                printf("0.0");
            }
            else if((output < 0) && (output > -1)){
                printf("-%d.",(int)intPartOutput);
                while(outputZeroPadding-- > 0)
                    printf("0");
                printf("%d", (int)decPartOutput);
            }
            else{
                printf("%d.",(int)intPartOutput);
                while((outputZeroPadding--) && (decPartOutput !=0))
                    printf("0");
                printf("%d", (int)decPartOutput);
            }

            printf("\n");
    }
*/
    if(strcasecmp(words[1], "q") == 0){
                output = atan(arg1);
                decPartOutput = fabs(decimalPlaces * (modf(output, &intPartOutput)));
                if(decPartOutput == decimalPlaces){
                    decPartOutput = 0;
                    intPartOutput += 1;
                }

                dummy = decPartOutput;
                while((dummy < decimalPlaces) && (decPartOutput !=0)){
                    dummy *=10;
                    outputZeroPadding++;
                }

                printf("function(");
                if((arg1 < 0) && (arg1 > -1)){
                    printf("-%d.",(int)intPartArg1);
                    while(arg1ZeroPadding-- > 0)
                        printf("0");
                    printf("%d", (int)decPartArg1);
                }
                else{
                    printf("%d.",(int)intPartArg1);
                    while((arg1ZeroPadding--) && (decPartArg1 !=0))
                        printf("0");
                    printf("%d", (int)decPartArg1);
                }
                printf(")");

                printf(" = ");
                //Zero case to deal with "negative" zero
                if(((int)intPartOutput == 0) && ((int)decPartOutput == 0)){
                    printf("0.0");
                }
                //Error to deal with infinity or negative infinity (a cap of 2 billion, 147 million)
                else if(((int)intPartOutput >= 2147000000) | ((int)intPartOutput <= -2147000000)){
                    printf("Error (too big or too small)");
                }
                else if((output < 0) && (output > -1)){
                    printf("-%d.",(int)intPartOutput);
                    while(outputZeroPadding-- > 0)
                        printf("0");
                    printf("%d", (int)decPartOutput);
                }
                else{
                    printf("%d.",(int)intPartOutput);
                    while((outputZeroPadding--) && (decPartOutput !=0))
                        printf("0");
                    printf("%d", (int)decPartOutput);
                }

                printf("\n");
        }

}


void projTestBench(void) {
    printf("Project testbench.\n");
    for(;;) {
        char buf[60];
        printf("> ");
        fgets(buf, sizeof buf - 1, stdin);
        int sz = strlen(buf);
        if (sz > 0)
            buf[sz-1] = '\0';
        char *words[7] = { 0,0,0,0,0,0,0 };
        int i;
        char *cp = buf;
        for(i=0; i<6; i++) {
            // strtok tokenizes a string, splitting it up into words that
            // are divided by any characters in the second argument.
            words[i] = strtok(cp," \t");
            // Once strtok() is initialized with the buffer,
            // subsequent calls should be made with NULL.
            cp = 0;
            if (words[i] == 0)
                break;
            if (i==0 && strcasecmp(words[0], "display1") == 0) {
                words[1] = strtok(cp, ""); // words[1] is rest of string
                break;
            }
            if (i==0 && strcasecmp(words[0], "display2") == 0) {
                words[1] = strtok(cp, ""); // words[1] is rest of string
                break;
            }
        }
        projAction(words);
    }
}

//The code for testing the outputing
/*
int main(void)
{
    tty_init();
    printf("This is my starting message\n");
    projTestBench();


    //prob3();
    //prob4();
    //prob5();
    //testbench();

    for(;;)
        asm("wfi");
}
*/
