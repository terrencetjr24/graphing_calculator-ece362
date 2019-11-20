#include <math.h>
#include <stdlib.h>
#include "parsing.h"

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

void init_stack(int* stack){
    int i;
    for(i =0; i< STACK_SIZE; i++){
        stack[i] = INIT_VALUE;
    }
    return;
}

//********************************************************************************
//Note: there are open parenthesis added at the end of certain functions
//********************************************************************************
int stackManipulation(int * stack, char adding){
    float answer;
    if((adding == 'A') | (adding == 'B')| (adding == 'C') | (adding == 'D')){
        switch(adding){
        case 'A':
            if(alternateFunc == ALTERNATE_1)
                alternateFunc = NORMAL;
            else
                alternateFunc = ALTERNATE_1;
            break;
        case 'B':
            if(alternateFunc == ALTERNATE_2)
                alternateFunc = NORMAL;
            else
                alternateFunc = ALTERNATE_2;
            break;
        case 'C':
            stackPointer = 0;
            //clearingScreenFunctiion();
            break;
        case 'D':
            if(stackCheck(stack))
                return 1;
            answer = calculations(stack);
            //outputFunction(answer);
            stackPointer =0;
            break;
        }
    }
    //Adding to the stack
    else{
        if(alternateFunc == NORMAL){
            switch(adding)
                {
                case '1': //
                    stack[stackPointer] = ONE;
                    break;
                case '2': //
                    stack[stackPointer] = TWO;
                    break;
                case '3': //
                    stack[stackPointer] = THREE;
                    break;
                case '4': //
                    stack[stackPointer] = FOUR;
                    break;
                case '5': //
                    stack[stackPointer] = FIVE;
                    break;
                case '6': //
                    stack[stackPointer] = SIX;
                    break;
                case '7': //
                    stack[stackPointer] = SEVEN;
                    break;
                case '8': //
                    stack[stackPointer] = EIGHT;
                    break;
                case '9': //
                    stack[stackPointer] = NINE;
                    break;
                case '*': //
                    stack[stackPointer] = DECIMAL;
                    break;
                case '0': //
                    stack[stackPointer] = ZERO;
                    break;
                case '#': //
                    stack[stackPointer] = NEGATIVE_SIGN;
                    break;
            }
        }
        else if (alternateFunc == ALTERNATE_1){
            switch(adding)
                    {
                    case '1': //
                        stack[stackPointer] = PLUS;
                        break;
                    case '2': //
                        stack[stackPointer] = MINUS;
                        break;
                    case '3': //
                        stack[stackPointer] = DIVIDE;
                        break;
                    case '4': //
                        stack[stackPointer] = MULTIPLY;
                        break;
                    case '5': //
                        //HAVE WE DECIDED WHAT TO DO WITH THESE YET
                        break;
                    case '6': //
                        //HAVE WE DECIDED WHAT TO DO WITH THESE YET
                        break;
                    case '7': //
                        stack[stackPointer] = E_TO_THE_X;
                        break;
                    case '8': //
                        stack[stackPointer++] = NATURAL_LOG;
                        stack[stackPointer] = OPEN_PAREN;
                        break;
                    case '9': //
                        stack[stackPointer++] = LOG10;
                        stack[stackPointer] = OPEN_PAREN;
                        break;
                    case '*': //
                        stack[stackPointer] = OPEN_PAREN;
                        break;
                    case '0': //
                        stack[stackPointer] = CARROT;
                        break;
                    case '#': //
                        stack[stackPointer] = CLOSE_PAREN;
                        break;
                    }
        }
        else{
            switch(adding)
                    {
                    case '1': //
                        stack[stackPointer++] = SIN;
                        stack[stackPointer] = OPEN_PAREN;
                        break;
                    case '2': //
                        stack[stackPointer++] = COS;
                        stack[stackPointer] = OPEN_PAREN;
                        break;
                    case '3': //
                        stack[stackPointer++] = TAN;
                        stack[stackPointer] = OPEN_PAREN;
                        break;
                    case '4': //
                        stack[stackPointer++] = ARCSIN;
                        stack[stackPointer] = OPEN_PAREN;
                        break;
                    case '5': //
                        stack[stackPointer++] = ARCCOS;
                        stack[stackPointer] = OPEN_PAREN;
                        break;
                    case '6': //
                        stack[stackPointer++] = ARCTAN;
                        stack[stackPointer] = OPEN_PAREN;
                        break;
                    case '7': //
                        stack[stackPointer] = FACTORIAL;
                        break;
                    case '8': //
                        stack[stackPointer] = X_VARIABLE;
                        break;
                    case '9': //
                        //HAVE WE DECIDED WHAT TO DO WITH THESE YET
                        break;
                    case '*': //
                        stack[stackPointer] = GRAPH;
                        //calling the graphing function and resetting stack pointer to zero
                        //graphing();
                        stackPointer = 0;
                        break;
                    case '0': //
                        //Will treat these specially within the stackCheck function
                        stack[stackPointer] = DEG_TO_RAD;
                        if(stackCheck(stack))
                            return 1;
                        answer = calculations(stack);
                        //outputFunc(answer);
                        stackPointer = 0;
                        break;
                    case '#': //
                        //Will treat these specially within the stackCheck function
                        stack[stackPointer] = RAD_TO_DEG;
                        if(stackCheck(stack))
                            return 1;
                        answer = calculations(stack);
                        //outputFunc(answer);
                        stackPointer = 0;
                        break;
                    }
        }
    stackPointer++;
    }
    return 0;
}

//Checking that:
//Open parens == closeing parens
//...

int stackCheck(int* stack){
    int openParens = 0;
    int closedParens = 0;

    //special case for if the last thing pressed was a RAD/DEG conversion
    if( (stack[stackPointer] == RAD_TO_DEG) || (stack[stackPointer] == DEG_TO_RAD) ){
        int x = 1;
    }
    else{

    }
    return 0;
}

float calculations(int* stack){
    float answer = 0;

    return answer;
}
