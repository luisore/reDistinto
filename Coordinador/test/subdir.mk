################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../test/test_distributor.c \
../test/test_suite.c 

O_SRCS += \
../test/test_distributor.o \
../test/test_suite.o 


OBJS += \
./test/test_distributor.o \
./test/test_suite.o 


C_DEPS += \
./test/test_distributor.d \
./test/test_suite.d 


# Each subdirectory must supply rules for building sources it contributes
test/%.o: ../test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


