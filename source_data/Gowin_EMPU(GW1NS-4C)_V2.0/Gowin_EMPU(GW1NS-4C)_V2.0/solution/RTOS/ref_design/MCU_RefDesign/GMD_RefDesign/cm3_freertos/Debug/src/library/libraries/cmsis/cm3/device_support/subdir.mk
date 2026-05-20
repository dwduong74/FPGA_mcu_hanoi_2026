################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/library/libraries/cmsis/cm3/device_support/system_gw1ns4c.c 

OBJS += \
./src/library/libraries/cmsis/cm3/device_support/system_gw1ns4c.o 

C_DEPS += \
./src/library/libraries/cmsis/cm3/device_support/system_gw1ns4c.d 


# Each subdirectory must supply rules for building sources it contributes
src/library/libraries/cmsis/cm3/device_support/%.o: ../src/library/libraries/cmsis/cm3/device_support/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\libraries\cmsis\cm3\core_support\gmd" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\libraries\cmsis\cm3\device_support" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\libraries\drivers\inc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\middlewares\gpio" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\middlewares\uart" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\middlewares\3rd_party\freertos\include" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\middlewares\3rd_party\freertos\portable\GMD\cm3" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\project\freertos\inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


