#include <math.h> //For the calculations
#include <stdlib.h>
#include <stdio.h>
#include <string.h> //For memset (clearing the struct)
#include "parsing.h"
#include <stdint.h>

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

//Parsing global variables
uint8_t alternateFunc = NORMAL;
uint8_t stackPointer = 0;

/* Note:
 * Will be creating 2 different stack within these sets of functions:
 * one for holding the codes from all the inputs (an int stack)
 * and another for holding the final numbers and the operations to take place
 *
 *It should be fine if these are in addition to the "display stack",
 *so long as it doesn't take up too much memory
 *
 */

float calculations(uint8_t* stack){
    float answer = 0;
    uint8_t workingPointer = 0;
    uint8_t secondaryPointer;
    uint8_t calcStackPointer = 0;
    struct calulationObj calcStack[STACK_SIZE];
    memset(&calcStack, 0, sizeof(calcStack));
    //The stack pointer will be the size of objects in the stack, and the next available location
    //A loop for creating the calcStack
    while(workingPointer != stackPointer){
        //If the value is a digit
        if((stack[workingPointer] <= NEGATIVE_SIGN) && (stack[workingPointer] > -1)){
            secondaryPointer = workingPointer;
            while((stack[secondaryPointer] < 12) && (secondaryPointer < stackPointer))
                secondaryPointer++;
            //Just for simulating
            //printf("beg val: %d , end val: %d\n", workingPointer, secondaryPointer-1);
            calcStack[calcStackPointer].number = convertToNum(stack, workingPointer, secondaryPointer-1);
            //Just for simulating
            //printf("The number 'read': %f\n", calcStack[calcStackPointer].number);
            calcStack[calcStackPointer++].numOrCode = 0;
            workingPointer = secondaryPointer-1;
        }
        else{
            calcStack[calcStackPointer].code = stack[workingPointer];
            //Assigning the precedence of each operator
            int operator = stack[workingPointer];
            if((operator == PLUS) | (operator == MINUS))
                calcStack[calcStackPointer++].numOrCode = 1;
            else if ((operator == DIVIDE) | (operator == MULTIPLY))
                calcStack[calcStackPointer++].numOrCode = 2;
            else if (operator == CARROT)
                calcStack[calcStackPointer++].numOrCode = 3;
            else if (operator == OPEN_PAREN)
                calcStack[calcStackPointer++].numOrCode = 4;
            else if (operator == CLOSE_PAREN)
                calcStack[calcStackPointer++].numOrCode = 5;
            //Nothing else should be expected here other than functions (to be safe could make this else if)
            else {
                calcStack[calcStackPointer++].numOrCode = 6;
            }
        }
        //printf("Iteration: %d\n", workingPointer);
        workingPointer++;
    }
    //Just for checking the calcStack (simulation)
    //printf("The calc stack: \n");
    //printCalcStack(calcStack, calcStackPointer);

    //THE ACTUAL ALGORITHM

    struct calulationObj outputQueue[STACK_SIZE];
    int indexOQ=0;
    struct calulationObj operatorStack[STACK_SIZE];
    int indexOS=0;
    memset(&outputQueue, 0, sizeof(outputQueue));
    memset(&operatorStack, 0, sizeof(operatorStack));
    workingPointer = 0;
    while(workingPointer != calcStackPointer){
        //Number
        if(calcStack[workingPointer].numOrCode == 0){
            memcpy(&(outputQueue[indexOQ++]), &(calcStack[workingPointer]), sizeof(struct calulationObj) );
        }
        //Function
        else if(calcStack[workingPointer].numOrCode == 6){
             memcpy(&(operatorStack[indexOS++]), &(calcStack[workingPointer]), sizeof(struct calulationObj) );
        }
        //A left paranthesis
        else if(calcStack[workingPointer].numOrCode == 4){
            memcpy(&(operatorStack[indexOS++]), &(calcStack[workingPointer]), sizeof(struct calulationObj) );
        }
        //A right paranthesis
        else if(calcStack[workingPointer].numOrCode == 5){
            //Do stuff
            while(operatorStack[indexOS-1].numOrCode != 4){
                memcpy(&(outputQueue[indexOQ++]), &(operatorStack[indexOS-1]), sizeof(struct calulationObj) );
                indexOS--;
            }
            //Popping the left paranthesis from the operator stack and "discarding it"
            indexOS--;
        }
        //Operator (could use an else if here to be more safe)
        else{
            //while (there's a function on top of stack, or operator on stack with higher precedence,
            // or the operator at top of stack has equal precedence and is left associative [assumption: carrots are the only right associative operator that we have])
            //AND the operator at the top of the stack is NOT a left paranthesis
            //Pop operators from the operator stack to the output queue
            while((indexOS > 0) && (operatorStack[indexOS-1].numOrCode != 4) && ( (operatorStack[indexOS-1].numOrCode == 5) | (operatorStack[indexOS-1].numOrCode > calcStack[workingPointer].numOrCode) | ((operatorStack[indexOS-1].numOrCode == calcStack[workingPointer].numOrCode) && (operatorStack[indexOS-1].code != CARROT) )) ){
                memcpy(&(outputQueue[indexOQ++]), &(operatorStack[indexOS-1]), sizeof(struct calulationObj) );
                indexOS--;
            }
            //Now... Pushing it to the operator stack
            memcpy(&(operatorStack[indexOS++]), &(calcStack[workingPointer]), sizeof(struct calulationObj) );
        }
        workingPointer++;
    }
    //After the while loop pop everything from the operator stack to the output queue
    while(indexOS !=0){
        memcpy(&(outputQueue[indexOQ++]), &(operatorStack[indexOS-1]), sizeof(struct calulationObj) );
        indexOS--;
    }
    //printf("The outputQueue size: %d\n", indexOQ);
    //printf("The output queue: \n");
    //printCalcStack(outputQueue, indexOQ);

    workingPointer = 0;
    secondaryPointer = 0;
    int thirdPointer = 0;
    float holder = 0;
    while(workingPointer != indexOQ){
        if((outputQueue[workingPointer].numOrCode == 0) | (outputQueue[workingPointer].dead == 1))
            asm("nop"); //This might not work lol
        //ACTUAL CALCULATIONS!!!
        else{
            switch(outputQueue[workingPointer].code){
                case PLUS:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    thirdPointer = secondaryPointer - 1;
                    while(outputQueue[thirdPointer].dead == 1)
                        thirdPointer--;
                    //Doing calculation
                    holder = outputQueue[thirdPointer].number + outputQueue[secondaryPointer].number;
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    outputQueue[thirdPointer].dead = 1;
                    break;
                case MINUS:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    thirdPointer = secondaryPointer - 1;
                    while(outputQueue[thirdPointer].dead == 1)
                        thirdPointer--;
                    //Doing calculation
                    holder = outputQueue[thirdPointer].number - outputQueue[secondaryPointer].number;
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    outputQueue[thirdPointer].dead = 1;
                    break;
                case DIVIDE:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    thirdPointer = secondaryPointer - 1;
                    while(outputQueue[thirdPointer].dead == 1)
                        thirdPointer--;
                    //Doing calculation
                    holder = outputQueue[thirdPointer].number / outputQueue[secondaryPointer].number;
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    outputQueue[thirdPointer].dead = 1;
                    break;
                case MULTIPLY:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    thirdPointer = secondaryPointer - 1;
                    while(outputQueue[thirdPointer].dead == 1)
                        thirdPointer--;
                    //Doing calculation
                    holder = outputQueue[thirdPointer].number * outputQueue[secondaryPointer].number;
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    outputQueue[thirdPointer].dead = 1;
                    break;
                case E_TO_THE_X:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = exp(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
                case NATURAL_LOG:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = log(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
                case LOG10:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = log10(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
                case CARROT:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    thirdPointer = secondaryPointer - 1;
                    while(outputQueue[thirdPointer].dead == 1)
                        thirdPointer--;
                    //Doing calculation
                    holder = pow(outputQueue[thirdPointer].number, outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    outputQueue[thirdPointer].dead = 1;
                    break;
                case SIN:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = sin(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
                case COS:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = cos(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
                case TAN:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = tan(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
                case ARCSIN:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = asin(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
                case ARCCOS:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = acos(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
                case ARCTAN:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = atan(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
                case FACTORIAL:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = factorial(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
            }
        }
        //printf("At position [%d] The holder value thus far: %f\n",workingPointer, holder);
        holder = 0;
        workingPointer++;
    }
    //The last value in the queue will hold the final value
    answer = outputQueue[indexOQ-1].number;
    return answer;
}

//********************************************************************************
//Note: there are open parenthesis added at the end of certain functions
//This is the only function that needs to be called by the button pressing function
//or the interrupt
//
//The function will call the "output-to-screen" function after D is pressed and the calculations are done
//Return Value: will return 0 if everything went fine
//                   return 1 if there's an error (and we need to beep at someone)
//********************************************************************************
uint8_t stackManipulation(uint8_t * stack, char* expression, uint8_t * index, char* result, char adding){
    float answer;
    int tempIndex = *index;
    char tempString[6] = "      ";
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
            *index = 1;
            //A return value of 3 will indicate a clear
            return 3;
            break;
        case 'D':
            if(stackPointer!= 0){
            if(stackCheck(stack, stackPointer)){
                stackPointer =0;
                return 1;
            }
            answer = calculations(stack);
            /********************************************************
             * Need to find a way to incorprate checking for if the output is -0,
             * or if the output is too big (maybe this isn't an issue tho)
             */
            sprintf(result, "%f", answer);

            stackPointer =0;
            return 2;
            }
            break;
        }
    }
    //Adding to the stack
    else{
        if(tempIndex >= 42){}
        else{
            tempIndex++;
            switch(adding)
                {
                case '1': //
                    if(alternateFunc == NORMAL){
                        stack[stackPointer++] = ONE;
                        strcpy(tempString,"1    ");
                    }
                    else if(alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = PLUS;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"+    ");
                    }
                    else {
                        stack[stackPointer++] = SIN;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"sin( ");
                        tempIndex+=3;
                    }
                    break;
                case '2': //
                    if(alternateFunc == NORMAL){
                        stack[stackPointer++] = TWO;
                        strcpy(tempString,"2    ");
                    }
                    else if (alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = MINUS;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"-    ");
                    }
                    else {
                        stack[stackPointer++] = COS;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"cos( ");
                        tempIndex+=3;
                    }
                    break;
                case '3': //
                    if(alternateFunc == NORMAL){
                        stack[stackPointer++] = THREE;
                        strcpy(tempString,"3    ");
                    }
                    else if(alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = DIVIDE;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"/    ");
                    }
                    else{
                        stack[stackPointer++] = TAN;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"tan( ");
                        tempIndex+=3;
                    }
                    break;
                case '4': //
                    if(alternateFunc == NORMAL){
                        stack[stackPointer++] = FOUR;
                        strcpy(tempString,"4    ");
                    }
                    else if(alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = MULTIPLY;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"*    ");
                    }
                    else {
                        stack[stackPointer++] = ARCSIN;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"asin(");
                        tempIndex+=4;
                    }
                    break;
                case '5': //
                    if(alternateFunc == NORMAL){
                    stack[stackPointer++] = FIVE;
                    strcpy(tempString,"5    ");
                    }
                    else if(alternateFunc == ALTERNATE_1){
                        asm("nop");
                        tempIndex--;
                        alternateFunc = NORMAL;
                    }
                    else{
                        stack[stackPointer++] = ARCCOS;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"acos(");
                        tempIndex+=4;
                    }
                    break;
                case '6': //
                    if(alternateFunc == NORMAL){
                        stack[stackPointer++] = SIX;
                        strcpy(tempString,"6    ");
                    }
                    else if(alternateFunc == ALTERNATE_1){
                        asm("nop");
                        tempIndex--;
                        alternateFunc = NORMAL;
                    }
                    else{
                        stack[stackPointer++] = ARCTAN;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"atan(");
                        tempIndex+=4;
                    }
                    break;
                case '7': //
                    if(alternateFunc == NORMAL){
                        stack[stackPointer++] = SEVEN;
                        strcpy(tempString,"7    ");
                    }
                    else if(alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = E_TO_THE_X;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"e^X  ");
                        tempIndex+=2;
                    }
                    else{
                        stack[stackPointer++] = FACTORIAL;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"!(   ");
                        tempIndex+=1;
                    }
                    break;
                case '8': //
                    if(alternateFunc == NORMAL){
                        stack[stackPointer++] = EIGHT;
                        strcpy(tempString,"8    ");
                    }
                    else if(alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = NATURAL_LOG;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"ln(  ");
                        tempIndex+=2;
                    }
                    else{
                        //THIS IS probably unnecessary here since it's only for graphing mode
                        stack[stackPointer++] = X_VARIABLE;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"X    ");
                    }
                    break;
                case '9': //
                    if(alternateFunc == NORMAL){
                        stack[stackPointer++] = NINE;
                        strcpy(tempString,"9    ");
                    }
                    else if(alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = LOG10;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"log( ");
                        tempIndex+=3;
                    }
                    else{
                        asm("nop");
                        tempIndex--;
                        alternateFunc = NORMAL;
                    }
                    break;
                case '*': //
                    if(alternateFunc == NORMAL){
                        //PROVISION TO DISALLOW DECIMALS NEXT TO EACH OTHER
                        if(stack[stackPointer-1] != DECIMAL){
                            stack[stackPointer++] = DECIMAL;
                            strcpy(tempString,".    ");
                        }
                    }
                    else if(alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"(    ");
                    }
                    else{
                        stack[stackPointer] = GRAPH;
                        //calling the graphing function and resetting stack pointer to zero
                        stackPointer = 0;
                        //graphing();
                        alternateFunc = NORMAL;
                    }
                    break;
                case '0': //
                    if(alternateFunc == NORMAL){
                        stack[stackPointer++] = ZERO;
                        strcpy(tempString,"0    ");
                    }
                    else if(alternateFunc == ALTERNATE_1){
                        //PROVISION TO DISALLOW CARROTS NEXT TO EACH OTHER
                        if(stack[stackPointer-1] != CARROT)
                            stack[stackPointer++] = CARROT;
                        alternateFunc = NORMAL;
                        strcpy(tempString,"^    ");
                    }
                    else{
                        //Will treat these specially within the stackCheck function (also DON'T WANT TO
                        stack[stackPointer] = DEG_TO_RAD;
                        if(stackCheck(stack, stackPointer))
                            return 1;
                        answer = degToRad(convertToNum(stack, 0, stackPointer-1));
                        //outputFunc(answer);
                        stackPointer = 0;
                        alternateFunc = NORMAL;
                    }
                    break;
                case '#': //
                    if(alternateFunc == NORMAL){
                    stack[stackPointer++] = NEGATIVE_SIGN;
                    strcpy(tempString,"-    ");
                    }
                    else if(alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = CLOSE_PAREN;
                        alternateFunc = NORMAL;
                        strcpy(tempString,")    ");
                    }
                    else{
                        //Will treat these specially within the stackCheck function
                        stack[stackPointer] = RAD_TO_DEG;
                        if(stackCheck(stack, stackPointer))
                            return 1;
                        answer = radToDeg(convertToNum(stack, 0, stackPointer-1));
                        //outputFunc(answer);
                        stackPointer = 0;
                        alternateFunc = NORMAL;
                    }
                    break;
            }
        }
        memcpy(&(expression[*index]),tempString,6);
        *index = tempIndex;
    }
    return 0;
}

