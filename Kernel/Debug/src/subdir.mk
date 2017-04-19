################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Configuration.c \
../src/ConnectionCore.c \
../src/InternalSocketFunctions.c \
../src/Kernel.c \
../src/KernelConfiguration.c \
../src/Listen.c \
../src/Process.c \
../src/SocketLibrary.c 

OBJS += \
./src/Configuration.o \
./src/ConnectionCore.o \
./src/InternalSocketFunctions.o \
./src/Kernel.o \
./src/KernelConfiguration.o \
./src/Listen.o \
./src/Process.o \
./src/SocketLibrary.o 

C_DEPS += \
./src/Configuration.d \
./src/ConnectionCore.d \
./src/InternalSocketFunctions.d \
./src/Kernel.d \
./src/KernelConfiguration.d \
./src/Listen.d \
./src/Process.d \
./src/SocketLibrary.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


