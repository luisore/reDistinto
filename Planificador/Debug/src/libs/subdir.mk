################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/libs/protocols.c \
../src/libs/serialize.c \
../src/libs/tcpserver.c 

O_SRCS += \
../src/libs/protocols.o \
../src/libs/serialize.o \
../src/libs/tcpserver.o 

OBJS += \
./src/libs/protocols.o \
./src/libs/serialize.o \
./src/libs/tcpserver.o 

C_DEPS += \
./src/libs/protocols.d \
./src/libs/serialize.d \
./src/libs/tcpserver.d 


# Each subdirectory must supply rules for building sources it contributes
src/libs/%.o: ../src/libs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


