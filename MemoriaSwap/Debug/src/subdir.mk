################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/MemoriaSwap.c \
../src/Swap.c \
../src/algoritmosReemplazo.c \
../src/estructuras.c \
../src/gestionMemoria.c \
../src/peticiones.c \
../src/utils.c 

OBJS += \
./src/MemoriaSwap.o \
./src/Swap.o \
./src/algoritmosReemplazo.o \
./src/estructuras.o \
./src/gestionMemoria.o \
./src/peticiones.o \
./src/utils.o 

C_DEPS += \
./src/MemoriaSwap.d \
./src/Swap.d \
./src/algoritmosReemplazo.d \
./src/estructuras.d \
./src/gestionMemoria.d \
./src/peticiones.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2022-1c-PaguenNuestroPsicologo/psicoLibrary" -I"/home/utnso/tp-2022-1c-PaguenNuestroPsicologo/psicoLibrary/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


