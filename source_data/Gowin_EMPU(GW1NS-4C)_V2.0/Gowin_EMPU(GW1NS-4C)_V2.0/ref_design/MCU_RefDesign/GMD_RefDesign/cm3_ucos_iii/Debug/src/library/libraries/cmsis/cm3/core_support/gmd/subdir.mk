################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/library/libraries/cmsis/cm3/core_support/gmd/core_cm3.c 

OBJS += \
./src/library/libraries/cmsis/cm3/core_support/gmd/core_cm3.o 

C_DEPS += \
./src/library/libraries/cmsis/cm3/core_support/gmd/core_cm3.d 


# Each subdirectory must supply rules for building sources it contributes
src/library/libraries/cmsis/cm3/core_support/gmd/%.o: ../src/library/libraries/cmsis/cm3/core_support/gmd/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\libraries\cmsis\cm3\core_support\gmd" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\project\ucos_iii\inc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\libraries\cmsis\cm3\device_support" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\libraries\drivers\inc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\gpio" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\uart" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\BSP" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\CONFIG" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\CPU" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\CPU\cm3\GMD" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\LIB" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\OS\Source" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\OS\Ports\cm3\Generic\GMD" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


