################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/library/middlewares/delay/delay.c 

OBJS += \
./src/library/middlewares/delay/delay.o 

C_DEPS += \
./src/library/middlewares/delay/delay.d 


# Each subdirectory must supply rules for building sources it contributes
src/library/middlewares/delay/%.o: ../src/library/middlewares/delay/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\libraries\cmsis\cm3\core_support\gmd" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\libraries\cmsis\cm3\device_support" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\libraries\drivers\inc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\delay" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\dmm" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\gpio" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\hyper_ram" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\psram" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\spi_flash" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\library\middlewares\uart" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_demo\src\project" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


