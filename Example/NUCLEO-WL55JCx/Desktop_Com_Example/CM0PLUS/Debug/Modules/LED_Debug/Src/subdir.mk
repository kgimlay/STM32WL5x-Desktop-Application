################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Modules/LED_Debug/Src/led_debug.c 

OBJS += \
./Modules/LED_Debug/Src/led_debug.o 

C_DEPS += \
./Modules/LED_Debug/Src/led_debug.d 


# Each subdirectory must supply rules for building sources it contributes
Modules/LED_Debug/Src/%.o Modules/LED_Debug/Src/%.su Modules/LED_Debug/Src/%.cyclo: ../Modules/LED_Debug/Src/%.c Modules/LED_Debug/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DCORE_CM0PLUS -DSTM32WL55xx -DUSE_HAL_DRIVER -c -I../Core/Inc -I../Modules/LED_Debug/Inc -I../Modules/Desktop_Communication/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Modules-2f-LED_Debug-2f-Src

clean-Modules-2f-LED_Debug-2f-Src:
	-$(RM) ./Modules/LED_Debug/Src/led_debug.cyclo ./Modules/LED_Debug/Src/led_debug.d ./Modules/LED_Debug/Src/led_debug.o ./Modules/LED_Debug/Src/led_debug.su

.PHONY: clean-Modules-2f-LED_Debug-2f-Src

