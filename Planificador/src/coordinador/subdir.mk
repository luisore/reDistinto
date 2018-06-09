################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/coordinador/Coordinador.c 

OBJS += \
./src/coordinador/Coordinador.o 

C_DEPS += \
./src/coordinador/Coordinador.d 


# Each subdirectory must supply rules for building sources it contributes
src/libs/%.o: ../src/coordinador/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


