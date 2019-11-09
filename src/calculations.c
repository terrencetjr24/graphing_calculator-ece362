//Calculation source file
//Author: Terrence Randall
//Description: Source file holding the functions of the math operations that will be done
//(File may later be broken into 2 or 3 files to increase read-ability)

//The basic functions may not be necessary unless we need to manipulate
//the output to format in some particular way, or for read-ability

//Test if the math library is working
#include <math.h>

void calculatorTest();

void calculatorTest(){


    printf("The output is: \n");
    return;
}


//Basic integer functions (if dividing between int and float proves inefficient, delete these)
int intAdd(int, int);
int intSub(int, int);
int intMul(int, int);
int intDiv(int, int);
//Basic float functions
float floatAdd(float, float);
float floatSub(float, float);
float floatMul(float, float);
float floatDiv(float, float);

int intAdd(int a, int b){
    return a+b;
}

int intSub(int a, int b){
    return a-b;
}

int intMul(int a, int b){
    return a*b;
}

int intDiv(int a, int b){
    return a/b;
}

float floatAdd(float a, float b){
    return a+b;
}

float floatSub(float a, float b){
    return a-b;
}

float floatMul(float a, float b){
    return a*b;
}

float floatDiv(float a, float b){
    return a/b;
}

