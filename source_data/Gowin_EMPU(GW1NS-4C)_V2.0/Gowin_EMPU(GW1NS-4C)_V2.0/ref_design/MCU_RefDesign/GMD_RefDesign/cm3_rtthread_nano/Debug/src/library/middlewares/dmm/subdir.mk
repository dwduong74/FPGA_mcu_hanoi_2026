################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/library/middlewares/dmm/malloc.c 

OBJS += \
./src/library/middlewares/dmm/malloc.o 

C_DEPS += \
./src/library/middlewares/dmm/malloc.d 


# Each subdirectory must supply rules for building sources it contributes
src/library/middlewares/dmm/%.o: ../src/library/middlewares/dmm/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\libraries\cmsis\cm3\core_support\gmd" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\libraries\cmsis\cm3\device_support" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\libraries\drivers\inc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\gpio" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\uart" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\3rd_party\rtthread_nano\bsp\cm3" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\3rd_party\rtthread_nano\components\finsh" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\3rd_party\rtthread_nano\include" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\3rd_party\rtthread_nano\include\libc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\project\rtthread_nano\inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


