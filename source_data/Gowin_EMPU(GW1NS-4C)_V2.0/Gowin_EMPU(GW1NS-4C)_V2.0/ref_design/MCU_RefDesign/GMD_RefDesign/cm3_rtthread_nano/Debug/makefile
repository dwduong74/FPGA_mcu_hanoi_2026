################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/project/rtthread_nano/src/subdir.mk
-include src/library/middlewares/uart/gmd/subdir.mk
-include src/library/middlewares/uart/subdir.mk
-include src/library/middlewares/spi_flash/subdir.mk
-include src/library/middlewares/psram/subdir.mk
-include src/library/middlewares/hyper_ram/subdir.mk
-include src/library/middlewares/gpio/subdir.mk
-include src/library/middlewares/dmm/subdir.mk
-include src/library/middlewares/delay/subdir.mk
-include src/library/middlewares/3rd_party/rtthread_nano/src/subdir.mk
-include src/library/middlewares/3rd_party/rtthread_nano/libcpu/arm/cortex-m3/subdir.mk
-include src/library/middlewares/3rd_party/rtthread_nano/components/finsh/subdir.mk
-include src/library/middlewares/3rd_party/rtthread_nano/components/device/subdir.mk
-include src/library/middlewares/3rd_party/rtthread_nano/bsp/cm3/drivers/subdir.mk
-include src/library/libraries/drivers/src/subdir.mk
-include src/library/libraries/cmsis/cm3/device_support/startup/gmd/subdir.mk
-include src/library/libraries/cmsis/cm3/device_support/subdir.mk
-include src/library/libraries/cmsis/cm3/core_support/gmd/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
cm3_rtthread_nano.bin \

SECONDARY_SIZE += \
cm3_rtthread_nano.siz \


# All Target
all: cm3_rtthread_nano.elf secondary-outputs

# Tool invocations
cm3_rtthread_nano.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM Cross C Linker'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -T "D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\libraries\cmsis\cm3\device_support\startup\gmd\linker\gw1ns4c_flash.ld" -Xlinker --gc-sections -Wl,-Map,"cm3_rtthread_nano.map" -o "cm3_rtthread_nano.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

cm3_rtthread_nano.bin: cm3_rtthread_nano.elf
	@echo 'Invoking: GNU ARM Cross Create Flash Image'
	arm-none-eabi-objcopy -O binary "cm3_rtthread_nano.elf"  "cm3_rtthread_nano.bin"
	@echo 'Finished building: $@'
	@echo ' '

cm3_rtthread_nano.siz: cm3_rtthread_nano.elf
	@echo 'Invoking: GNU ARM Cross Print Size'
	arm-none-eabi-size --format=berkeley "cm3_rtthread_nano.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) cm3_rtthread_nano.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
