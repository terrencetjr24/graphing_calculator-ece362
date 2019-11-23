#include <math.h> //For the calculations
#include <stdlib.h>
#include <string.h> //For memset (clearing the struct)
#include "parsing.h"

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

//Parsing global variables
int alternateFunc = NORMAL;
int stackPointer = 0;

/*
 * Will be creating 2 different stack within these sets of functions:
 * one for holding the codes from all the inputs (an int stack)
 * and another for holding the final numbers and the operations to take place
 *
 *It should be fine if these are in addition to the "display stack",
 *so long as it doesn't take up too much memory
 *
 */

float calculations(int* stack){
    float answer = 0;
    int workingPointer = 0;
    int secondaryPointer;
    int calcStackPointer = 0;
    struct calulationObj calcStack[STACK_SIZE];
    memset(&calcStack, INIT_VALUE, sizeof(calcStack));
    //The stack pointer will be the size of objects in the stack, and the next available location
    //A loop for creating the calcStack
    while(workingPointer != stackPointer){
        //If the value is a digit
        if((stack[workingPointer] <= NEGATIVE_SIGN) && (stack[workingPointer] > -1)){
            secondaryPointer = workingPointer;
            while((stack[secondaryPointer] < 12) && (secondaryPointer < stackPointer))
                secondaryPointer++;
            //printf("beg val: %d , end val: %d\n", workingPointer, secondaryPointer-1);
            calcStack[calcStackPointer].number = convertToNum(stack, workingPointer, secondaryPointer-1);
            //printf("The number 'read': %f\n", calcStack[calcStackPointer].number); //Just to check
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
            //
            //******************************************
            //NEED TO ADD MORE ELSE IFs here
            //to COVER MORE OF THE PRECEDENCE CASES
            //*****************************************
            //
        }
        //printf("Iteration: %d\n", workingPointer);
        workingPointer++;
    }
    //Just for checking the calcStack (simulation)
    //printCalcStack(calcStack, calcStackPointer);

    //THE ACTUAL ALGORITHM

    //These are both only arrays of indices, and the actual values will be accessed from the calcStack (can make char to reduce space)
    int outputQueue[STACK_SIZE];
    int indexOQ=0;
    int operatorStack[STACK_SIZE];
    int indexOS=0;
    memset(&outputQueue, INIT_VALUE, sizeof(outputQueue));
    memset(&operatorStack, INIT_VALUE, sizeof(operatorStack));
    workingPointer = 0;

    while(workingPointer != calcStackPointer){
        //Number
        if(calcStack[workingPointer].numOrCode == 0){
            outputQueue[indexOQ++] = workingPointer;
            workingPointer++
        }
        //Function
        else if(){

        }
        //Operator
        else if(){

        }
    }

    return answer;
}

void init_stack(int* stack){
    int i;
    for(i =0; i< STACK_SIZE; i++){
        stack[i] = INIT_VALUE;
    }
    return;
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
                    stack[stackPointer++] = ONE;
                    break;
                case '2': //
                    stack[stackPointer++] = TWO;
                    break;
                case '3': //
                    stack[stackPointer++] = THREE;
                    break;
                case '4': //
                    stack[stackPointer++] = FOUR;
                    break;
                case '5': //
                    stack[stackPointer++] = FIVE;
                    break;
                case '6': //
                    stack[stackPointer++] = SIX;
                    break;
                case '7': //
                    stack[stackPointer++] = SEVEN;
                    break;
                case '8': //
                    stack[stackPointer++] = EIGHT;
                    break;
                case '9': //
                    stack[stackPointer++] = NINE;
                    break;
                case '*': //
                    //PROVISION TO DISALLOW DECIMALS NEXT TO EACH OTHER
                    if(stack[stackPointer-1] != DECIMAL)
                        stack[stackPointer++] = DECIMAL;
                    break;
                case '0': //
                    stack[stackPointer++] = ZERO;
                    break;
                case '#': //
                    stack[stackPointer++] = NEGATIVE_SIGN;
                    break;
            }
        }
        else if (alternateFunc == ALTERNATE_1){
            switch(adding)
                    {
                    case '1': //
                        stack[stackPointer++] = PLUS;
                        alternateFunc = NORMAL;
                        break;
                    case '2': //
                        stack[stackPointer++] = MINUS;
                        alternateFunc = NORMAL;
                        break;
                    case '3': //
                        stack[stackPointer++] = DIVIDE;
                        alternateFunc = NORMAL;
                        break;
                    case '4': //
                        stack[stackPointer++] = MULTIPLY;
                        alternateFunc = NORMAL;
                        break;
                    case '5': //
                        //HAVE WE DECIDED WHAT TO DO WITH THESE YET
                        break;
                    case '6': //
                        //HAVE WE DECIDED WHAT TO DO WITH THESE YET
                        break;
                    case '7': //
                        stack[stackPointer++] = E_TO_THE_X;
                        alternateFunc = NORMAL;
                        break;
                    case '8': //
                        stack[stackPointer++] = NATURAL_LOG;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        break;
                    case '9': //
                        stack[stackPointer++] = LOG10;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        break;
                    case '*': //
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        break;
                    case '0': //
                        //PROVISION TO DISALLOW CARROTS NEXT TO EACH OTHER
                        if(stack[stackPointer-1] != CARROT)
                            stack[stackPointer++] = CARROT;
                        alternateFunc = NORMAL;
                        break;
                    case '#': //
                        stack[stackPointer++] = CLOSE_PAREN;
                        alternateFunc = NORMAL;
                        break;
                    }
        }
        else{
            switch(adding)
                    {
                    case '1': //
                        stack[stackPointer++] = SIN;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        break;
                    case '2': //
                        stack[stackPointer++] = COS;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        break;
                    case '3': //
                        stack[stackPointer++] = TAN;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        break;
                    case '4': //
                        stack[stackPointer++] = ARCSIN;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        break;
                    case '5': //
                        stack[stackPointer++] = ARCCOS;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        break;
                    case '6': //
                        stack[stackPointer++] = ARCTAN;
                        stack[stackPointer++] = OPEN_PAREN;
                        alternateFunc = NORMAL;
                        break;
                    case '7': //
                        //PROVISION TO DISALLOW FACTORALS NEXT TO EACH OTHER
                        if(stack[stackPointer-1] != FACTORIAL)
                            stack[stackPointer++] = FACTORIAL;
                        alternateFunc = NORMAL;
                        break;
                    case '8': //
                        //THIS IS probably unnecessary here since it's only for graphing mode
                        stack[stackPointer++] = X_VARIABLE;
                        alternateFunc = NORMAL;
                        break;
                    case '9': //
                        //HAVE WE DECIDED WHAT TO DO WITH THESE YET
                        break;
                    case '*': //
                        stack[stackPointer] = GRAPH;
                        //calling the graphing function and resetting stack pointer to zero
                        stackPointer = 0;
                        //graphing();
                        alternateFunc = NORMAL;
                        break;
                    case '0': //
                        //Will treat these specially within the stackCheck function (also DON'T WANT TO
                        stack[stackPointer] = DEG_TO_RAD;
                        if(stackCheck(stack))
                            return 1;
                        //answer = calculations(stack);
                        //outputFunc(answer);
                        stackPointer = 0;
                        alternateFunc = NORMAL;
                        break;
                    case '#': //
                        //Will treat these specially within the stackCheck function
                        stack[stackPointer] = RAD_TO_DEG;
                        if(stackCheck(stack))
                            return 1;
                        //answer = calculations(stack);
                        //outputFunc(answer);
                        stackPointer = 0;
                        alternateFunc = NORMAL;
                        break;
                    }
        }
    }
    return 0;
}

//Checking that:
//Open parens == closing parens
//...
int stackCheck(int* stack){
    int openParens = 0;
    int closedParens = 0;
    int i;

    //special case for if the last thing pressed was a RAD/DEG conversion
    if( (stack[stackPointer] == RAD_TO_DEG) || (stack[stackPointer] == DEG_TO_RAD) ){
        for(i=0; i<stackPointer; i++){
            if(stack[i] > 11) //if there's anything other than numbers, decimals, or negative sin BEFORE the conversion operation
                return 1;
        }
    }
    else{
        //Checking for different cases here
        for(i=0; i<=stackPointer; i++){
            if(stack[i] == OPEN_PAREN)
                openParens++;
            else if(stack[i] == CLOSE_PAREN)
                closedParens++;
        }
    }

    if(openParens != closedParens)
        return 1;
    return 0;
}

//**********************************************
//This function works well enough, but it also produces slight rounding errors
//that may throw some calculations off
//**********************************************
float convertToNum(int* stack, int beg, int end){
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
        printf("The beg equals the ends\n");
        return returnVal;
    }
    //returning it to zero since it wasn't needed earlier
    returnVal = 0;

    int i;
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

//In this configuration would need a line like this in the output call
/*
 * int code = floatToString(dummy, output);
    if(code == 1)
        gcvt(dummy, 5, output);
    else if (code >1)
        gcvt(dummy, code + 4, output);
 *
 *Otherwise we can just uncomment the "gcvt()" calls within the function below
 *(But I'm unsure if this will work, since it doesn't work in the simulator)
 */

int floatToString(float input, char* output){
    int decimalPlaces = 100000;
    int digitsBeforeDec = 0;
    double intPartInput = 0;
    double decPartInput = 0;
    int holder;

    //I forget what this is doing, but I know it is very much essential
    decPartInput = fabs(decimalPlaces * (modf(input, &intPartInput)));
    intPartInput = fabs(intPartInput);
    if(decPartInput == decimalPlaces){
        decPartInput = 0;
        intPartInput += 1;
    }
    //Figuring out the number of digits before decimal point
    holder = (int)intPartInput;
    while(holder){
        digitsBeforeDec++;
        holder = holder /10;
    }
    //printf("The digits before decimal: %d\n", digitsBeforeDec);

    //Zero case to deal with "negative" zero
    if(((int)intPartInput == 0) && ((int)decPartInput == 0)){
        sprintf(output,"0.0");
        return 0;
    }
    //Error to deal with infinity or negative infinity (a cap of 2 billion, 147 million)
    else if(((int)intPartInput >= 2147000000) | ((int)intPartInput <= -2147000000)){
        sprintf(output, "Error (too big or too small)");
        return 0;
    }
    //A number is -0.####
    else if((input < 0) && (input > -1)){
        //gcvt(input, 5, output);
        return 1;
    }
    //Every other number
    else{
        //gcvt(input, beforeDecimal + 4, output);
        return digitsBeforeDec+1; //Just in case there's a positve 0.## number
    }
}

/*
void floatToString(float input, char* output){
    int outIndex = 0;
    int decimalPlaces = 100000;
    float dummy;
    int digitsBeforeDec = 0;
    int digitsAfterDec = 0;
    int holder;

    double intPartInput = 0;
    double decPartInput = 0;
    int inputZeroPadding = -1;
    //I forget what this is doing, but I know it is very much essential
    decPartInput = fabs(decimalPlaces * (modf(input, &intPartInput)));
    intPartInput = fabs(intPartInput);
    if(decPartInput == decimalPlaces){
        decPartInput = 0;
        intPartInput += 1;
    }
    //Figuring out the number of zeros to pad after the decimal
    dummy = decPartInput;
    while((dummy < decimalPlaces) && (decPartInput !=0)){
        dummy *=10;
        inputZeroPadding++;
    }
    //Figuring out the number of digits before and after decimal point
        holder = (int)intPartInput;
        while(holder){
            digitsBeforeDec++;
            holder = holder /10;
        }
        holder = (int)decPartInput;
        while(holder){
            //This is to make sure we don't add unnecessary zeros at the end
            if((holder) != ((holder/10) * 10))
                digitsAfterDec++;
            holder = holder / 10;
        }
        printf("The digits before decimal: %d\nAfter the decimal: %d", digitsBeforeDec, digitsAfterDec);

    //Zero case to deal with "negative" zero
    if(((int)intPartInput == 0) && ((int)decPartInput == 0)){
        sprintf(output,"0.0");
        gcvt(0.0, 2, output);
    }
    //Error to deal with infinity or negative infinity (a cap of 2 billion, 147 million)
    else if(((int)intPartInput >= 2147000000) | ((int)intPartInput <= -2147000000)){
        sprintf(output, "Error (too big or too small)");
    }
    else if((input < 0) && (input > -1)){
        output[outIndex++] = '-';
        output[outIndex++] = '0';
        output[outIndex++] = '.';
        //Printing all the padding values
        while(inputZeroPadding-- > 0){
            output[outIndex++] = '0';
        }
        //Printing all the values after the decimal
        while(digitsAfterDec){
            output[outIndex++] = '0' + (int)intPartInput / (pow(10, digitsAfterDec));
            digitsAfterDec--;
        }
        output[outIndex] = '\0';
        gcvt(input, 5, output);
    }
    else{
        //Printing a negatie sign if the value is negative
        if(input < 0)
            output[outIndex++] = '-';
        //Printing all the values before the decimal
        while(digitsBeforeDec){
            output[outIndex++] = '0' + (int)intPartInput / (pow(10, digitsBeforeDec));
            digitsBeforeDec--;
        }
        output[outIndex++] = '.';
        //Printing out the padding zeros
        while((inputZeroPadding--) && (decPartInput !=0))
            output[outIndex++] = '0';
        while(digitsAfterDec){
            output[outIndex++] = '0' + (int)intPartInput / (pow(10, digitsAfterDec));
            digitsAfterDec--;
        }
        output[outIndex] = '\0';
        gcvt(dummy, beforeDecimal + 5, output);
    }
    return;
}
*/
