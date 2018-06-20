################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../library/protocols.c \
../library/serialize.c \
../library/tcpserver.c 

OBJS += \
./library/protocols.o \
./library/serialize.o \
./library/tcpserver.o 

C_DEPS += \
./library/protocols.d \
./library/serialize.d \
./library/tcpserver.d 


# Each subdirectory must supply rules for building sources it contributes
library/%.o: ../library/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


