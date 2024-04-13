################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/DeviceManager.c \
../Src/LCD.c \
../Src/Queue.c \
../Src/USART.c \
../Src/Utils.c \
../Src/main.c \
../Src/syscalls.c \
../Src/sysmem.c 

OBJS += \
./Src/DeviceManager.o \
./Src/LCD.o \
./Src/Queue.o \
./Src/USART.o \
./Src/Utils.o \
./Src/main.o \
./Src/syscalls.o \
./Src/sysmem.o 

C_DEPS += \
./Src/DeviceManager.d \
./Src/LCD.d \
./Src/Queue.d \
./Src/USART.d \
./Src/Utils.d \
./Src/main.d \
./Src/syscalls.d \
./Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F401RE -DSTM32 -DSTM32F401RETx -DSTM32F4 -c -I../Inc -I"D:/Funix/IOT302 - Tong quan ve IOT & Lap trinh C nhung cho vi dieu khien/SDK_1.0.3_NUCLEO-F401RE/shared/Drivers/CMSIS/Include" -I"D:/Funix/IOT302 - Tong quan ve IOT & Lap trinh C nhung cho vi dieu khien/SDK_1.0.3_NUCLEO-F401RE/shared/Drivers/STM32F401RE_StdPeriph_Driver/inc" -I"D:/Funix/IOT302 - Tong quan ve IOT & Lap trinh C nhung cho vi dieu khien/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/button" -I"D:/Funix/IOT302 - Tong quan ve IOT & Lap trinh C nhung cho vi dieu khien/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/buzzer" -I"D:/Funix/IOT302 - Tong quan ve IOT & Lap trinh C nhung cho vi dieu khien/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/led" -I"D:/Funix/IOT302 - Tong quan ve IOT & Lap trinh C nhung cho vi dieu khien/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/rtos" -I"D:/Funix/IOT302 - Tong quan ve IOT & Lap trinh C nhung cho vi dieu khien/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/sensor" -I"D:/Funix/IOT302 - Tong quan ve IOT & Lap trinh C nhung cho vi dieu khien/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/serial" -I"D:/Funix/IOT302 - Tong quan ve IOT & Lap trinh C nhung cho vi dieu khien/SDK_1.0.3_NUCLEO-F401RE/shared/Middle/ucglib" -I"D:/Funix/IOT302 - Tong quan ve IOT & Lap trinh C nhung cho vi dieu khien/SDK_1.0.3_NUCLEO-F401RE/shared/Utilities" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/DeviceManager.cyclo ./Src/DeviceManager.d ./Src/DeviceManager.o ./Src/DeviceManager.su ./Src/LCD.cyclo ./Src/LCD.d ./Src/LCD.o ./Src/LCD.su ./Src/Queue.cyclo ./Src/Queue.d ./Src/Queue.o ./Src/Queue.su ./Src/USART.cyclo ./Src/USART.d ./Src/USART.o ./Src/USART.su ./Src/Utils.cyclo ./Src/Utils.d ./Src/Utils.o ./Src/Utils.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su

.PHONY: clean-Src

