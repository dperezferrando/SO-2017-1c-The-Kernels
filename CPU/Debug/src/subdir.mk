################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/CPU.c \
../src/Configuration.c \
../src/InternalSocketFunctions.c \
../src/SocketLibrary.c 

OBJS += \
./src/CPU.o \
./src/Configuration.o \
./src/InternalSocketFunctions.o \
./src/SocketLibrary.o 

C_DEPS += \
./src/CPU.d \
./src/Configuration.d \
./src/InternalSocketFunctions.d \
./src/SocketLibrary.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


