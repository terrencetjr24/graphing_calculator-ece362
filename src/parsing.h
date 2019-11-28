/*
 * parsing.h
 *
 *  Created on: Nov 19, 2019
 *      Author: terrencerandall
 */

#ifndef PARSING_H_
#define PARSING_H_

//BUTTON CODES
#define INIT_VALUE      -1
#define ZERO            0
#define ONE             1
#define TWO             2
#define THREE           3
#define FOUR            4
#define FIVE            5
#define SIX             6
#define SEVEN           7
#define EIGHT           8
#define NINE            9
#define DECIMAL         10
#define NEGATIVE_SIGN   11
#define PLUS            12
#define MINUS           13
#define DIVIDE          14
#define MULTIPLY        15
#define E_TO_THE_X      16
#define NATURAL_LOG     17
#define LOG10           18
#define CARROT          19
#define OPEN_PAREN      20
#define CLOSE_PAREN     21
#define SIN             22
#define COS             23
#define TAN             24
#define ARCSIN          25
#define ARCCOS          26
#define ARCTAN          27
//TREATING THIS AS A FUNCTION
#define FACTORIAL       28

#define X_VARIABLE      29
#define GRAPH           30
//Treated specially, don't need to be accounted for in the calculation switch statement of parsing
#define DEG_TO_RAD      31
#define RAD_TO_DEG      32

//Added late
#define PI 33

//STATUS CODE
#define NORMAL          0
#define ALTERNATE_1     1
#define ALTERNATE_2     2

//Stack value
#define STACK_SIZE 40

//Notes:
/*We can try to get around the factorial issue (and just treat it specially) to allow for inputs like (2+(3!))
 * or we can continue to treat is as a single input function (eg. Fact(35) )
 * Further, DegToRad & RadToDeg are currently being treated as functions that will only be
 * carried out on a single number after their respective buttons are pressed
 * (see the bottom of "stackManipulation()"), but they can be made into functions as well with more work
 *
 */

//Calculations structure

struct calulationObj
{
   float number;
   uint8_t code;
   uint8_t numOrCode;  //If the value is a 0 it's a number,
                    //if it's greater than 0 that's it's precedence (higher == higher precedence)
                    //Also functions will be DENOTED W/ A six (func. == 6)
   uint8_t dead;       //This will be used in the output queue during calculations
                    //1 == DEAD ; 0 == ALIVE
};


//Function declarations
//allowing us to add "things" to stack (number, operations, functions) [returns a 1 for error, 0 for no error]
uint8_t stackManipulation(uint8_t * stack, char* expression, uint8_t * index, char* result, char adding, uint8_t* altFunc, int* bkspStack, int* bkspIndex);
//Called within above function to check the validity of an input [returns a 1 for error, 0 for no error]
uint8_t stackCheck(uint8_t* stack, uint8_t pointer);
//used to calculate the result after enter is pressed (this is where everything is going on)
float calculations(uint8_t* stack);
//Used for converting a part of the stack to a number
float convertToNum(uint8_t* stack, uint8_t firstIndex, uint8_t lastIndex);

double factorial(double);
double radToDeg(double input);
double degToRad(double input);
void graphingFunc(float * inputArray, float * outputArray, uint16_t domain);
void micro_wait(unsigned int);

#endif /* PARSING_H_ */

