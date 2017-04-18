################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ClientePrueba.c \
../src/InternalSocketFunctions.c \
../src/SocketLibrary.c 

OBJS += \
./src/ClientePrueba.o \
./src/InternalSocketFunctions.o \
./src/SocketLibrary.o 

C_DEPS += \
./src/ClientePrueba.d \
./src/InternalSocketFunctions.d \
./src/SocketLibrary.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


