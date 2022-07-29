################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/CPU.c \
../src/MMU.c \
../src/tlb.c \
../src/utils.c 

OBJS += \
./src/CPU.o \
./src/MMU.o \
./src/tlb.o \
./src/utils.o 

C_DEPS += \
./src/CPU.d \
./src/MMU.d \
./src/tlb.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2022-1c-PaguenNuestroPsicologo/psicoLibrary" -I"/home/utnso/tp-2022-1c-PaguenNuestroPsicologo/psicoLibrary/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


