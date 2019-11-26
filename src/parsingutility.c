#include <math.h> //For the calculations
#include <stdlib.h>
#include <stdio.h>
#include <string.h> //For memset (clearing the struct)
#include "parsing.h"
#include <stdint.h>

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

/*
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
        //printf("The beg equals the ends\n"); //just for simulation
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

//Checking that:
//Open parens == closing parens
//...
uint8_t stackCheck(uint8_t* stack, uint8_t stackPointer){
    uint8_t openParens = 0;
    uint8_t closedParens = 0;
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
*/
