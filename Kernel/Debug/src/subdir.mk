################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/CapaFileSystem.c \
../src/Configuration.c \
../src/ConnectionCore.c \
../src/ConsolaKernel.c \
../src/Kernel.c \
../src/KernelConfiguration.c \
../src/Listen.c \
../src/Process.c \
../src/ProcessTest.c \
../src/SocketLibrary.c \
../src/heapManagementTest.c \
../src/kernelTest.c 

OBJS += \
./src/CapaFileSystem.o \
./src/Configuration.o \
./src/ConnectionCore.o \
./src/ConsolaKernel.o \
./src/Kernel.o \
./src/KernelConfiguration.o \
./src/Listen.o \
./src/Process.o \
./src/ProcessTest.o \
./src/SocketLibrary.o \
./src/heapManagementTest.o \
./src/kernelTest.o 

C_DEPS += \
./src/CapaFileSystem.d \
./src/Configuration.d \
./src/ConnectionCore.d \
./src/ConsolaKernel.d \
./src/Kernel.d \
./src/KernelConfiguration.d \
./src/Listen.d \
./src/Process.d \
./src/ProcessTest.d \
./src/SocketLibrary.d \
./src/heapManagementTest.d \
./src/kernelTest.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


