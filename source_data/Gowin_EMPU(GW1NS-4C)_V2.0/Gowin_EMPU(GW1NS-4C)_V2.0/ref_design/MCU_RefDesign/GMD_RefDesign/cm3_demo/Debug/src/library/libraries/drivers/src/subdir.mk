################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/library/libraries/drivers/src/gw1ns4c_gpio.c \
../src/library/libraries/drivers/src/gw1ns4c_i2c.c \
../src/library/libraries/drivers/src/gw1ns4c_misc.c \
../src/library/libraries/drivers/src/gw1ns4c_rtc.c \
../src/library/libraries/drivers/src/gw1ns4c_spi.c \
../src/library/libraries/drivers/src/gw1ns4c_syscon.c \
../src/library/libraries/drivers/src/gw1ns4c_timer.c \
../src/library/libraries/drivers/src/gw1ns4c_uart.c \
../src/library/libraries/drivers/src/gw1ns4c_wdog.c 

OBJS += \
./src/library/libraries/drivers/src/gw1ns4c_gpio.o \
./src/library/libraries/drivers/src/gw1ns4c_i2c.o \
./src/library/libraries/drivers/src/gw1ns4c_misc.o \
./src/library/libraries/drivers/src/gw1ns4c_rtc.o \
./src/library/libraries/drivers/src/gw1ns4c_spi.o \
./src/library/libraries/drivers/src/gw1ns4c_syscon.o \
./src/library/libraries/drivers/src/gw1ns4c_timer.o \
./src/library/libraries/drivers/src/gw1ns4c_uart.o \
./src/library/libraries/drivers/src/gw1ns4c_wdog.o 

C_DEPS += \
./src/library/libraries/drivers/src/gw1ns4c_gpio.d \
./src/library/libraries/drivers/src/gw1ns4c_i2c.d \
./src/library/libraries/drivers/src/gw1ns4c_misc.d \
./src/library/libraries/drivers/src/gw1ns4c_rtc.d \
./src/library/libraries/drivers/src/gw1ns4c_spi.d \
./src/library/libraries/drivers/src/gw1ns4c_syscon.d \
./src/library/libraries/drivers/src/gw1ns4c_timer.d \
./src/library/libraries/drivers/src/gw1ns4c_uart.d \
./src/library/libraries/drivers/src/gw1ns4c_wdog.d 


# Each subdirectory must supply rules for building sources it contributes
src/library/libraries/drivers/src/%.o: ../src/library/libraries/drivers/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\libraries\cmsis\cm3\core_support\gmd" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\libraries\cmsis\cm3\device_support" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\libraries\drivers\inc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\delay" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\dmm" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\gpio" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\hyper_ram" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\psram" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\spi_flash" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\uart" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\project" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


