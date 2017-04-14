################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Configuration.c \
../src/Consola.c \
../src/InternalSocketFunctions.c \
../src/SocketLibrary.c 

OBJS += \
./src/Configuration.o \
./src/Consola.o \
./src/InternalSocketFunctions.o \
./src/SocketLibrary.o 

C_DEPS += \
./src/Configuration.d \
./src/Consola.d \
./src/InternalSocketFunctions.d \
./src/SocketLibrary.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


