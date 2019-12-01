#include <math.h> //For the calculations
#include <stdlib.h> // can we remove any of these libraries
#include <stdio.h>
#include <string.h> //For memset (clearing the struct)
#include "parsing.h"
#include <stdint.h>

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

//Parsing global variables
uint8_t stackPointer = 0;
float answer;// = NULL;

/* Note:
 * Will be creating 2 different stack within these sets of functions:
 * one for holding the codes from all the inputs (an int stack)
 * and another for holding the final numbers and the operations to take place
 *
 *It should be fine if these are in addition to the "display stack",
 *so long as it doesn't take up too much memory
 *
 */

int getGraphValue(){
    char key;
    char displayString[5] = {0};
    displayString[0] = '+';
    uint8_t digitCount = 1;
    int domainInput = 0;
    int lastAdded;
    while(digitCount < 4){
        key = get_char_key();
        backspace:
        switch (key){
        case '0':
            lastAdded = 0 * (100/pow(10,(digitCount-1)));
            domainInput += 0 * (100/pow(10,(digitCount-1)));
            displayString[digitCount] = '0';
            break;
        case '1':
            lastAdded = 1 * (100/pow(10,(digitCount-1)));
            domainInput += 1 * (100/pow(10,(digitCount-1)));
            displayString[digitCount] = '1';
            break;
        case '2':
            lastAdded = 2 * (100/pow(10,(digitCount-1)));
            domainInput += 2 * (100/pow(10,(digitCount-1)));
            displayString[digitCount] = '2';
            break;
        case '3':
            lastAdded = 3 * (100/pow(10,(digitCount-1)));
            domainInput += 3 * (100/pow(10,(digitCount-1)));
            displayString[digitCount] = '3';
            break;
        case '4':
            lastAdded = 4 * (100/pow(10,(digitCount-1)));
            domainInput += 4 * (100/pow(10,(digitCount-1)));
            displayString[digitCount] = '4';
            break;
        case '5':
            lastAdded = 5 * (100/pow(10,(digitCount-1)));
            domainInput += 5 * (100/pow(10,(digitCount-1)));
            displayString[digitCount] = '5';
            break;
        case '6':
            lastAdded = 6 * (100/pow(10,(digitCount-1)));
            domainInput += 6 * (100/pow(10,(digitCount-1)));
            displayString[digitCount] = '6';
            break;
        case '7':
            lastAdded = 7 * (100/pow(10,(digitCount-1)));
            domainInput += 7 * (100/pow(10,(digitCount-1)));
            displayString[digitCount] = '7';
            break;
        case '8':
            lastAdded = 8 * (100/pow(10,(digitCount-1)));
            domainInput += 8 * (100/pow(10,(digitCount-1)));
            displayString[digitCount] = '8';
            break;
        case '9':
            lastAdded = 9 * (100/pow(10,(digitCount-1)));
            domainInput += 9 * (100/pow(10,(digitCount-1)));
            displayString[digitCount] = '9';
            break;
        case '#':
            if(domainInput > 0) displayString[0] = '-';
            else displayString[0] = '+';
            domainInput = domainInput * -1;
            digitCount--;
            break;
        case 'C':
            if(digitCount > 1) {
                displayString[--digitCount] = ' ';
                domainInput -= lastAdded;
                digitCount--;
            }
            break;
        default:
            digitCount--;
            break;
        }
        digitCount++;
        GLCD_GoTo(0,5);
        GLCD_WriteString(displayString);
    }
    while((key = get_char_key()) != 'D') {
        if(key == 'C'){
            goto backspace;
        }
        if(key == '#'){
            if(domainInput > 0) displayString[0] = '-';
            else displayString[0] = '+';
            domainInput = domainInput * -1;
        }
        GLCD_GoTo(0,5);
        GLCD_WriteString(displayString);
    }

    return domainInput;
}

float calculations(uint8_t* stack){
    uint8_t graphing = 0;
    uint8_t workingPointer = 0;
    uint8_t secondaryPointer;
    uint8_t calcStackPointer = 0;
    struct calulationObj calcStack[STACK_SIZE];
    memset(&calcStack, 0, sizeof(calcStack));

    //graphing variables
    uint8_t xcount = 0;
    uint8_t xIndicies[STACK_SIZE/4];
    int xmin = 0;
    int xmax = 0;
    int ymin = 0;
    int ymax = 0;
    uint8_t i;
    float outputArray[128];
    float inputArray[128];
    memset (outputArray, 0, sizeof outputArray);
    memset (inputArray, 0, sizeof inputArray);
    memset(xIndicies, -1, sizeof xIndicies);
    char dummy[100];

    //The stack pointer will be the size of objects in the stack, and the next available location
    //A loop for creating the calcStack
    while(workingPointer != stackPointer){
        //If the value is a digit
        if(((stack[workingPointer] <= NEGATIVE_SIGN) && (stack[workingPointer] > -1)) | (stack[workingPointer] == PI) | (stack[workingPointer] == X_VARIABLE) | (stack[workingPointer] == PREV_ANS)){
            if(stack[workingPointer] == PI){
                //Had to reduce precision to allow sin(PI) to work
                calcStack[calcStackPointer].number = 3.1415;
                calcStack[calcStackPointer++].numOrCode = 0;
            }
            else if(stack[workingPointer] == X_VARIABLE){
                calcStack[calcStackPointer].number = -1;
                calcStack[calcStackPointer].numOrCode = 0;
                calcStack[calcStackPointer++].code = stack[workingPointer];
            }
            else if(stack[workingPointer] == PREV_ANS){
                calcStack[calcStackPointer].number = answer;
                calcStack[calcStackPointer++].numOrCode = 0;
            }
            else{
                secondaryPointer = workingPointer;
                while((stack[secondaryPointer] < 12) && (secondaryPointer < stackPointer))
                    secondaryPointer++;
                calcStack[calcStackPointer].number = convertToNum(stack, workingPointer, secondaryPointer-1);
                calcStack[calcStackPointer++].numOrCode = 0;
                workingPointer = secondaryPointer-1;
            }
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
        workingPointer++;
    }

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
            //Popping the left parenthesis from the operator stack and "discarding it"
            indexOS--;
        }
        //Operator (could use an else if here to be more safe)
        else{
            //while (there's a function on top of stack, or operator on stack with higher precedence,
            // or the operator at top of stack has equal precedence and is left associative [assumption: carrots are the only right associative operator that we have])
            //AND the operator at the top of the stack is NOT a left parenthesis
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

    //Checking that we're in graphing mode
    if(outputQueue[indexOQ-1].code == GRAPH){
        graphing = 1;
        indexOQ--; //getting rid of the graph function, since I know what to do

        //Count the number of Xs in the function, and save their indicies
        //If there are zero then I'll be able to output a constant value, but I'll first need to check
        //the input to make sure there aren't any input errors
        workingPointer = 0;
        while(workingPointer != indexOQ){
            if(outputQueue[workingPointer].code == X_VARIABLE) {
                xIndicies[xcount] = workingPointer;
                xcount++;
            }
            workingPointer++;
        }

        //Send a message to screen telling the user to enter a three digit domain that will be mirrored
        //Call get_key_pressed() to evaluate what this is (make sure to ignore everything except for numbers)
        //Then convert the input to a number

        GLCD_GoTo(0,2);
        GLCD_WriteString("                     ");
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

        GLCD_GoTo(0,3);
        GLCD_WriteString("Enter 3-digit xmin");
        GLCD_GoTo(0,4);
        GLCD_WriteString("then press D");

        xmin = getGraphValue();

        GLCD_GoTo(0,5);
        GLCD_WriteString("                     ");
        GLCD_GoTo(0,3);
        GLCD_WriteString("Enter 3-digit xmax");
        GLCD_GoTo(0,4);
        GLCD_WriteString("then press D");

        xmax = getGraphValue();

        GLCD_GoTo(0,5);
        GLCD_WriteString("                     ");
        GLCD_GoTo(0,3);
        GLCD_WriteString("Enter 3-digit ymin");
        GLCD_GoTo(0,4);
        GLCD_WriteString("then press D");

        ymin = getGraphValue();

        GLCD_GoTo(0,5);
        GLCD_WriteString("                     ");
        GLCD_GoTo(0,3);
        GLCD_WriteString("Enter 3-digit ymax");
        GLCD_GoTo(0,4);
        GLCD_WriteString("then press D");

        ymax = getGraphValue();

        //Getting the size of the domain
        float stepSize = (float)(xmin+xmax) / 128.0;

        inputArray[0] = xmin;
        for(i = 1; i< 128; i++){
            inputArray[i] = inputArray[i-1]+stepSize;
        }
        //Then with this array of 128 values, enter those into the X places of my function
        //and send the output to some graphing function
        //Note: this will be a for loop
    }
    //This will only loop through once if there's no X-Variable
    struct calulationObj originalOQ[STACK_SIZE];
    memcpy(originalOQ, outputQueue, sizeof outputQueue);
    uint8_t q;
    for(q=0; q <128; q++){
        memcpy(outputQueue, originalOQ, sizeof outputQueue);
        //Subing in the inpt value to the x locations
        for(i=0; i<xcount; i++){
            //This is a double check just to be sure
            if(outputQueue[xIndicies[i]].code == X_VARIABLE){
                outputQueue[xIndicies[i]].number = inputArray[q];
            }
        }

    workingPointer = 0;
    secondaryPointer = 0;
    uint8_t thirdPointer = 0;
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
                case DEG_TO_RAD:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = degToRad(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
                case RAD_TO_DEG:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Doing calculation
                    holder = radToDeg(outputQueue[secondaryPointer].number);
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
                case GRAPH:
                    secondaryPointer = workingPointer-1;
                    while(outputQueue[secondaryPointer].dead == 1)
                        secondaryPointer--;
                    //Just doing a reassignment here
                    holder = outputQueue[secondaryPointer].number;
                    //Putting output into the last queue position and changing it to a number
                    outputQueue[workingPointer].number = holder;
                    outputQueue[workingPointer].numOrCode = 0;
                    //"Killing" the operands
                    outputQueue[secondaryPointer].dead = 1;
                    break;
            }
        }
        holder = 0;
        workingPointer++;
    }
    //This will allow us to break out of the function if it's not a loop
    if(graphing == 0)
        break; //for breaking out of the for loop if the function is not graphing
    else
        outputArray[q] = outputQueue[indexOQ-1].number;
    }
    //The last value in the queue will hold the final value
    //And this won't matter if we're not graphing
    if(graphing){
        //call the graphing function and pass through the entire array
        graphingFunc(inputArray, outputArray, xmin, xmax, ymin, ymax);
        return 0;
    }

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
uint8_t stackManipulation(uint8_t * stack, char* expression, uint8_t * index, char* result, char adding, uint8_t* alternateFunc, int * bkspStack, int* bkspIndex){
    int tempIndex = *index;
    //float answer;
    char tempString[21] = "                     ";
    if((adding == 'A') | (adding == 'B')| (adding == 'C') | (adding == 'D')){
        switch(adding){
        case 'A':
            if(*alternateFunc == ALTERNATE_1)
                *alternateFunc = NORMAL;
            else
                *alternateFunc = ALTERNATE_1;
            break;
        case 'B':
            if(*alternateFunc == ALTERNATE_2)
                *alternateFunc = NORMAL;
            else
                *alternateFunc = ALTERNATE_2;
            break;
        case 'C':
            if(tempIndex > 1) stackPointer--;
            return 3;
            break;
        case 'D':
            (*bkspIndex) = 0;
            if(stackPointer!= 0){
                if(stackCheck(stack, stackPointer)){
                    stackPointer = 0;
                    sprintf(result, "Error invalid input");
                    answer = 0;
                    return 1;
                }
                answer = calculations(stack);

                if((answer > 2147000000) | (answer < -2147000000)){
                    sprintf(result, "Error, too big/small");
                    stackPointer = 0;
                    return 1;
                }
                else if((answer < 0) && (answer > -0.0001))
                    sprintf(result, "0.0");
                else
                    sprintf(result, "%.4f", answer);
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
                case '1':
                    bkspStack[(*bkspIndex)] = 1;
                    if(*alternateFunc == NORMAL){
                        stack[stackPointer++] = ONE;
                        strcpy(tempString,"1                   ");
                    }
                    else if(*alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = PLUS;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"+                   ");
                    }
                    else {
                        bkspStack[(*bkspIndex)] = 3;
                        bkspStack[++(*bkspIndex)] = 1;
                        stack[stackPointer++] = SIN;
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"sin(                ");
                        tempIndex+=3;
                    }
                    break;
                case '2':
                    bkspStack[(*bkspIndex)] = 1;
                    if(*alternateFunc == NORMAL){
                        stack[stackPointer++] = TWO;
                        strcpy(tempString,"2                   ");
                    }
                    else if (*alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = MINUS;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"-                   ");
                    }
                    else {
                        bkspStack[(*bkspIndex)] = 3;
                        bkspStack[++(*bkspIndex)] = 1;
                        stack[stackPointer++] = COS;
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"cos(                ");
                        tempIndex+=3;
                    }
                    break;
                case '3':
                    bkspStack[(*bkspIndex)] = 1;
                    if(*alternateFunc == NORMAL){
                        stack[stackPointer++] = THREE;
                        strcpy(tempString,"3                   ");
                    }
                    else if(*alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = DIVIDE;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"/                   ");
                    }
                    else{
                        bkspStack[(*bkspIndex)] = 3;
                        bkspStack[++(*bkspIndex)] = 1;
                        stack[stackPointer++] = TAN;
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"tan(                ");
                        tempIndex+=3;
                    }
                    break;
                case '4':
                    bkspStack[(*bkspIndex)] = 1;
                    if(*alternateFunc == NORMAL){
                        stack[stackPointer++] = FOUR;
                        strcpy(tempString,"4                   ");
                    }
                    else if(*alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = MULTIPLY;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"*                   ");
                    }
                    else {
                        bkspStack[(*bkspIndex)] = 4;
                        bkspStack[++(*bkspIndex)] = 1;
                        stack[stackPointer++] = ARCSIN;
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"asin(               ");
                        tempIndex+=4;
                    }
                    break;
                case '5':
                    if(*alternateFunc == NORMAL){
                        bkspStack[(*bkspIndex)] = 1;
                        stack[stackPointer++] = FIVE;
                        strcpy(tempString,"5                   ");
                    }
                    else if(*alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"(    ");
                    }
                    else{
                        bkspStack[(*bkspIndex)] = 4;
                        bkspStack[++(*bkspIndex)] = 1;
                        stack[stackPointer++] = ARCCOS;
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"acos(");
                        tempIndex+=4;
                    }
                    break;
                case '6': //
                    if(*alternateFunc == NORMAL){
                        bkspStack[(*bkspIndex)] = 1;
                        stack[stackPointer++] = SIX;
                        strcpy(tempString,"6    ");
                    }
                    else if(*alternateFunc == ALTERNATE_1){
                        stack[stackPointer++] = CLOSE_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,")    ");
                    }
                    else{
                        bkspStack[(*bkspIndex)] = 4;
                        bkspStack[++(*bkspIndex)] = 1;
                        stack[stackPointer++] = ARCTAN;
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"atan(");
                        tempIndex+=4;
                    }
                    break;
                case '7': //
                    if(*alternateFunc == NORMAL){
                        bkspStack[(*bkspIndex)] = 1;
                        stack[stackPointer++] = SEVEN;
                        strcpy(tempString,"7                   ");
                    }
                    else if(*alternateFunc == ALTERNATE_1){
                        bkspStack[(*bkspIndex)] = 2;
                        stack[stackPointer++] = E_TO_THE_X;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"e^                  ");
                        tempIndex+=1;
                    }
                    else{
                        bkspStack[(*bkspIndex)] = 1;
                        bkspStack[++(*bkspIndex)] = 1;
                        stack[stackPointer++] = FACTORIAL;
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"!(                  ");
                        tempIndex+=1;
                    }
                    break;
                case '8':
                    bkspStack[(*bkspIndex)] = 1;
                    if(*alternateFunc == NORMAL){
                        stack[stackPointer++] = EIGHT;
                        strcpy(tempString,"8                   ");
                    }
                    else if(*alternateFunc == ALTERNATE_1){
                        bkspStack[(*bkspIndex)] = 2;
                        bkspStack[++(*bkspIndex)] = 1;
                        stack[stackPointer++] = NATURAL_LOG;
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"ln(                 ");
                        tempIndex+=2;
                    }
                    else{
                        stack[stackPointer++] = X_VARIABLE;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"X                   ");
                    }
                    break;
                case '9': //
                    if(*alternateFunc == NORMAL){
                        bkspStack[(*bkspIndex)] = 1;
                        stack[stackPointer++] = NINE;
                        strcpy(tempString,"9                   ");
                    }
                    else if(*alternateFunc == ALTERNATE_1){
                        bkspStack[(*bkspIndex)] = 3;
                        bkspStack[++(*bkspIndex)] = 1;
                        stack[stackPointer++] = LOG10;
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"log(                ");
                        tempIndex+=3;
                    }
                    else{
                        bkspStack[(*bkspIndex)] = 2;
                        stack[stackPointer++] = PI;
                        strcpy(tempString,"PI                  ");
                        tempIndex++;
                        *alternateFunc = NORMAL;
                    }
                    break;
                case '*':
                    bkspStack[(*bkspIndex)] = 1;
                    if(*alternateFunc == NORMAL){
                        //PROVISION TO DISALLOW DECIMALS NEXT TO EACH OTHER
                        if(stack[stackPointer-1] != DECIMAL){
                            stack[stackPointer++] = DECIMAL;
                            strcpy(tempString,".                   ");
                        }
                        else
                            return 0;
                    }
                    else if(*alternateFunc == ALTERNATE_1){
                        *alternateFunc = NORMAL;
                        //PROVISION TO DISALLOW CARROTS NEXT TO EACH OTHER
                        if(stack[stackPointer-1] != CARROT){
                            stack[stackPointer++] = CARROT;
                            strcpy(tempString,"^                   ");
                        }
                        else
                            return 0;
                    }
                    else{
                        if(*index == 1) {
                            bkspStack[(*bkspIndex)] = 5;
                            bkspStack[++(*bkspIndex)] = 1;
                            stack[stackPointer++] = GRAPH;
                            stack[stackPointer++] = OPEN_PAREN;
                            *alternateFunc = NORMAL;
                            //Need the open parenthesis here to signal to the user
                            //that they need to close the parenthesis at the end of the input
                            //otherwise the "graph" function will not work like a function
                            strcpy(tempString,"f(x)=(              ");
                            tempIndex+=5;
                        }
                    }
                    break;
                case '0':
                    bkspStack[(*bkspIndex)] = 1;
                    if(*alternateFunc == NORMAL){
                        stack[stackPointer++] = ZERO;
                        strcpy(tempString,"0                   ");
                    }
                    //The clear button
                    else if(*alternateFunc == ALTERNATE_1){
                        stackPointer = 0;
                        tempIndex = 1;
                        memset(expression,32,66);
                        memset(expression,62,1);
                    }
                    else{
                        bkspStack[(*bkspIndex)] = 4;
                        bkspStack[++(*bkspIndex)] = 1;
                        stack[stackPointer++] = DEG_TO_RAD;
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"DtoR(               ");
                        tempIndex+=4;
                    }
                    break;
                case '#':
                    bkspStack[(*bkspIndex)] = 1;
                    if(*alternateFunc == NORMAL){
                        //PROVISION TO DISALLOW CARROTS NEXT TO EACH OTHER
                        if(stack[stackPointer-1] != NEGATIVE_SIGN){
                            stack[stackPointer++] = NEGATIVE_SIGN;
                            strcpy(tempString,"-                   ");
                        }
                    }
                    //Previous answer (for inserting into the function)
                    else if(*alternateFunc == ALTERNATE_1){
                        //if(answer != NULL) {
                            bkspStack[(*bkspIndex)] = 3;
                            stack[stackPointer++] = PREV_ANS;
                            *alternateFunc = NORMAL;
                            strcpy(tempString,"ANS                   ");
                            tempIndex+=2;
                        //}
                    }
                    else{
                        bkspStack[(*bkspIndex)] = 4;
                        bkspStack[++(*bkspIndex)] = 1;
                        stack[stackPointer++] = RAD_TO_DEG;
                        stack[stackPointer++] = OPEN_PAREN;
                        *alternateFunc = NORMAL;
                        strcpy(tempString,"RtoD(                 ");
                        tempIndex+=4;
                    }
                    break;
            }
            (*bkspIndex)++;
        }
        memcpy(&(expression[*index]),tempString,21);
        *index = tempIndex;
    }
    return 0;
}

void graphingFunc(float * inputArray, float * outputArray, int xmin, int xmax, int ymin, int ymax) {

    uint8_t q;
    uint8_t Nans = 0;
    for(q=0; q<128 ;q++){
        if( (!(outputArray[q] > 5)) && (!(outputArray[q] < 5)) && (!(outputArray[q] == 5)))
            Nans++;
    }
    if(Nans >= 64){
        //Print an error message to the screen indicating the error
        //Can either wait a second or two then automatically return, or wait for a button press
    }

    //micro_wait(10000000);
    GLCD_ClearScreen();

    //GLCD_GoTo(0,2);
    //GLCD_WriteString("it works 2");
    /*float ymax = outputArray[0];
    float ymin = outputArray[0];
    for (int j = 1; j < 128; j++)
    {
        if (ymin > outputArray[j])
        {
            ymin = outputArray[j];
        }
        if (ymax < outputArray[j])
        {
            ymax = outputArray[j];
        }
    }
    float ystep = (ymax - ymin) / 64;*/

    float yscreenmax = 10; //specified by user
    float yscreenmin = -10; //specified by user
    int yaxispos = 32; // middle position

    float xscreenmax = 0; //specified by user
    float xscreenmin = 0; //specified by user
    int xaxispos = 64; // middle position

    //Axes

    // x axis
    if (yscreenmin >= 0.0){
        yscreenmin = 0;
        yaxispos = 0;
    }
    else if (yscreenmax < 0.0){
        yscreenmax = 0;
        yaxispos = 63;
    }
    else
        yaxispos = -64 * yscreenmin / (yscreenmax - yscreenmin);

    // x axis
    for(int i = 0; i < 128; i++){
        //on(i,32);
        GLCD_SetPixel(i,yaxispos,'b');
    }

    // y axis
    if (xscreenmin > 0.0){
        xscreenmin = 0;
        xaxispos = 0;
    }
    else if (xscreenmax < 0.0){
        xscreenmax = 0;
        xaxispos = 127;
    }
    // y axis
    for(int i = 0; i < 64; i++){
        //on(64,i);
        GLCD_SetPixel(64,i,'b');
    }

    float ystep = (yscreenmax - yscreenmin) / 64;
    for (int xpix = 0; xpix < 128; xpix++){
        int ypix = (int) ((outputArray[xpix] - yscreenmin) / ystep);
        if (ypix >= 0 && ypix < 64){
            GLCD_SetPixel(xpix,ypix,'b');
        }
    }

    /*char dummy[100];
    sprintf(dummy,"%.4f", ymin);
    GLCD_GoTo(0,5);
    GLCD_WriteString(dummy);
    sprintf(dummy,"%.4f", ymax);
    GLCD_GoTo(0,6);
    GLCD_WriteString(dummy);
    sprintf(dummy,"%.4f", ystep);
    GLCD_GoTo(0,7);
    GLCD_WriteString(dummy);
    micro_wait(10000000);*/

    // Axes
    /*for(int i = 0; i < 128; i++)
    {
        //on(i,32);
        GLCD_SetPixel(i,32,'b');
    }
    for(int i = 0; i < 64; i++)
    {
        //on(64,i);
        GLCD_SetPixel(64,i,'b');
    }*/

    /*for (int xpix = 0; xpix < 128; xpix++)
    {
        int ypix = (int) ((outputArray[xpix] - ymin) / ystep);
        GLCD_SetPixel(xpix,ypix,'b');
    }*/

    while(get_char_key() != 'D');

    // Prompt for domain (at least max x - domain = (-x,x) )
    // graphing function should do the same as enter - basically,
    // loop from 0 to 127 and call eval function with X replaced by pixel #
    // need to prompt for domain first.
    GLCD_ClearScreen();
    //answer = calculations(stack);

}


//**********************************************
//This function works well enough, but it also produces slight rounding errors
//that may throw some calculations off
//**********************************************
float convertToNum(uint8_t* stack, uint8_t beg, uint8_t end){
    //Note:
    //If all errors in input were taken care of carefully, and the proper indices
    //are given, all the value in this range should be numbers.
    //And all the numbers are "DEFINED" with their actual number value
    int decimal = -1;
    int placesBeforeDec = 0;
    int placesAfterDec = 0;
    float returnVal = stack[beg];
    int workingIndex;

    if(beg == end){
        return returnVal;
    }
    //returning it to zero since it wasn't needed earlier
    returnVal = 0;

    uint8_t i;
    //Incrementing through the value in the stack to "find" a decimal and assign it's index
    for(i=beg; i<=end; i++){
        if(stack[i] == DECIMAL)
            decimal = i;
    }

    //If there's no decimal, treat the number as a whole number
    if(decimal == -1){
        placesBeforeDec = (end-beg) +1;
        //if the number is negative, decrement the number of digits and move the starting index up one
        if(stack[beg] == NEGATIVE_SIGN){
            placesBeforeDec--;
            i=beg+1;
        }
        else
            i=beg;

        while(placesBeforeDec >0){
            returnVal += stack[i++] * pow(10, placesBeforeDec-1);
            placesBeforeDec--;
        }
    }
    else{
        placesBeforeDec = decimal-beg;
        placesAfterDec = end-decimal;

        //if the number is negative, decrement the number of digits and move the starting index up one
        if(stack[beg] == NEGATIVE_SIGN){
            placesBeforeDec--;
            workingIndex = beg +1;
        }
        else
            workingIndex = beg;

        //Converting from the front of the number to the decimal
        while(workingIndex != decimal){
            returnVal += stack[workingIndex++] * pow(10, placesBeforeDec-1);
            placesBeforeDec--;
        }
        //Converting from the bag of the number to the decimal
        workingIndex = end;
        while(workingIndex != decimal){
            returnVal += stack[workingIndex--] * pow(0.1, placesAfterDec);
            placesAfterDec--;
        }
    }
    //If the number is negative return the negative version
    if(stack[beg] == NEGATIVE_SIGN)
        return (-1*returnVal);
    else
        return returnVal;
}

double radToDeg(double input){
  double output = (input * 180.0)/3.14159;
  return output;
}

double degToRad(double input){
  double output = (input * 3.14159) / 180.0;
  return output;
}

double factorial(double input){
    double output = 1;
    int limit = (int)input;
    int i;
    for(i=1; i<= limit; i++)
        output *= i;

    return output;
}

//Checking (in order) that:
//The last item in stack is a number or a closed parenthesis (or, that it's NOT an operator)
//Done in for loop: checking '(' == ')' , no operations next to one another, if not graphing and an X-Var is present
//Things blocked in the stack manipulations:
// decimals, carrots, and negative signs next to itself
uint8_t stackCheck(uint8_t* stack, uint8_t stackPointer){
    uint8_t openParens = 0;
    uint8_t closedParens = 0;
    uint8_t i;

    uint8_t prevCodeStaus = 0; //1 = operation, 2 = number, 3 = Constant, 4 = function, 5 = open parenthesis, 6 = closed parenthesis

    uint8_t graphing =0;

    //Checking that last value is a number, or closed parenthesis, or PI, or X_VARIALE , or PREV_ANS  (or just NOT an operator)
    //(a number can end with a decimal point [eg. 2.] but can't end in a negative)
    if(!((stack[stackPointer-1] >=ZERO && stack[stackPointer-1] <=DECIMAL) | (stack[stackPointer-1] == CLOSE_PAREN) | (stack[stackPointer-1] == PI) | (stack[stackPointer-1] == X_VARIABLE) | (stack[stackPointer-1] == PREV_ANS)))
        return 1;
    if(stack[0] == GRAPH)
        graphing =1;
    for(i=0; i<stackPointer; i++){
        //Checking for equal number of open an closed parenthesis
        if(stack[i] == OPEN_PAREN){
            openParens++;
            prevCodeStaus = 5;
        }
        else if(stack[i] == CLOSE_PAREN){
            if(prevCodeStaus == 1)
                return 1;
            closedParens++;
            prevCodeStaus = 6;
        }
        //Checking that no operators are next to one another, and that it wasn't a parenthesis
        else if(stack[i] >= PLUS && stack[i] <= MULTIPLY){
            if((prevCodeStaus == 1) | (prevCodeStaus == 5))
                return 1;
            else
                prevCodeStaus = 1;
        }
        //Checking if the previous thing is a number
        else if((stack[i] == PREV_ANS) | (stack[i] == PI)){
            if(prevCodeStaus == 2)
                return 1;
            prevCodeStaus = 3;
        }
        //If there's an X-Var, make sure that the function is graphing
        else if (stack[i] == X_VARIABLE){
            prevCodeStaus = 0;
            if(!graphing)
                return 1;
        }
        //Checking that numbers aren't placed directly behind constant values
        else if((stack[i] <= NEGATIVE_SIGN) & (stack[i] > -1)){
            if(prevCodeStaus == 3)
                return 1;
            prevCodeStaus = 2;
        }
        //Checking that functions aren't placed directly behind numbers or constants
        else if((stack[i] <= FACTORIAL) & (stack[i] >= E_TO_THE_X)){
            if((prevCodeStaus == 2) | (prevCodeStaus == 3))
                return 1;
            prevCodeStaus = 4;
        }
    }

    if(openParens != closedParens)
        return 1;

    return 0;
    return 0;
}
