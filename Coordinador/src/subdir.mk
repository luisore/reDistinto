################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Coordinador.c \
../src/Distributor.c 

O_SRCS += \
../src/Coordinador.o \
../src/Distributor.o

OBJS += \
./src/Coordinador.o \
./src/Distributor.o

C_DEPS += \
./src/Coordinador.d \
../src/Distributor.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


