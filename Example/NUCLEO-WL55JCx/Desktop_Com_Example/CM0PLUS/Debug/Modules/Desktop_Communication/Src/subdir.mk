################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Modules/Desktop_Communication/Src/desktop_app_session.c \
../Modules/Desktop_Communication/Src/uart_packet_helpers.c \
../Modules/Desktop_Communication/Src/uart_transport_layer.c 

OBJS += \
./Modules/Desktop_Communication/Src/desktop_app_session.o \
./Modules/Desktop_Communication/Src/uart_packet_helpers.o \
./Modules/Desktop_Communication/Src/uart_transport_layer.o 

C_DEPS += \
./Modules/Desktop_Communication/Src/desktop_app_session.d \
./Modules/Desktop_Communication/Src/uart_packet_helpers.d \
./Modules/Desktop_Communication/Src/uart_transport_layer.d 


# Each subdirectory must supply rules for building sources it contributes
Modules/Desktop_Communication/Src/%.o Modules/Desktop_Communication/Src/%.su Modules/Desktop_Communication/Src/%.cyclo: ../Modules/Desktop_Communication/Src/%.c Modules/Desktop_Communication/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DCORE_CM0PLUS -DSTM32WL55xx -DUSE_HAL_DRIVER -c -I../Core/Inc -I../Modules/LED_Debug/Inc -I../Modules/Desktop_Communication/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Modules-2f-Desktop_Communication-2f-Src

clean-Modules-2f-Desktop_Communication-2f-Src:
	-$(RM) ./Modules/Desktop_Communication/Src/desktop_app_session.cyclo ./Modules/Desktop_Communication/Src/desktop_app_session.d ./Modules/Desktop_Communication/Src/desktop_app_session.o ./Modules/Desktop_Communication/Src/desktop_app_session.su ./Modules/Desktop_Communication/Src/uart_packet_helpers.cyclo ./Modules/Desktop_Communication/Src/uart_packet_helpers.d ./Modules/Desktop_Communication/Src/uart_packet_helpers.o ./Modules/Desktop_Communication/Src/uart_packet_helpers.su ./Modules/Desktop_Communication/Src/uart_transport_layer.cyclo ./Modules/Desktop_Communication/Src/uart_transport_layer.d ./Modules/Desktop_Communication/Src/uart_transport_layer.o ./Modules/Desktop_Communication/Src/uart_transport_layer.su

.PHONY: clean-Modules-2f-Desktop_Communication-2f-Src

