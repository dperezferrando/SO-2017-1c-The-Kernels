################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Memoria.c 

OBJS += \
./src/Memoria.o 

C_DEPS += \
./src/Memoria.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
<<<<<<< HEAD
	gcc -include"/home/utnso/tp-2017-1c-The-Kernels/ConfigLibrary/src/Configuration.h" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
=======
	gcc -include"/home/utnso/Escritorio/tp-2017-1c-The-Kernels/ConfigLibrary/src/Configuration.h" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
>>>>>>> 633dc3827af2602ac61f156326da457cb10ba6ad
	@echo 'Finished building: $<'
	@echo ' '


