################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../src/micro_wait.s 

C_SRCS += \
../src/KS0108-STM32.c \
../src/KS0108.c \
../src/graphic.c \
../src/main.c \
../src/syscalls.c \
../src/system_stm32f0xx.c 

OBJS += \
./src/KS0108-STM32.o \
./src/KS0108.o \
./src/graphic.o \
./src/main.o \
./src/micro_wait.o \
./src/syscalls.o \
./src/system_stm32f0xx.o 

C_DEPS += \
./src/KS0108-STM32.d \
./src/KS0108.d \
./src/graphic.d \
./src/main.d \
./src/syscalls.d \
./src/system_stm32f0xx.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -mfloat-abi=soft -DSTM32 -DSTM32F0 -DSTM32F051R8Tx -DSTM32F0DISCOVERY -DDEBUG -DSTM32F051 -DUSE_STDPERIPH_DRIVER -I"/Users/praneethmedepalli/Documents/workspace/miniProj/Utilities" -I"/Users/praneethmedepalli/Documents/workspace/miniProj/StdPeriph_Driver/inc" -I"/Users/praneethmedepalli/Documents/workspace/miniProj/inc" -I"/Users/praneethmedepalli/Documents/workspace/miniProj/CMSIS/device" -I"/Users/praneethmedepalli/Documents/workspace/miniProj/CMSIS/core" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.s
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Assembler'
	@echo $(PWD)
	arm-none-eabi-as -mcpu=cortex-m0 -mthumb -mfloat-abi=soft -I"/Users/praneethmedepalli/Documents/workspace/miniProj/Utilities" -I"/Users/praneethmedepalli/Documents/workspace/miniProj/StdPeriph_Driver/inc" -I"/Users/praneethmedepalli/Documents/workspace/miniProj/inc" -I"/Users/praneethmedepalli/Documents/workspace/miniProj/CMSIS/device" -I"/Users/praneethmedepalli/Documents/workspace/miniProj/CMSIS/core" -g -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


