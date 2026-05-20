################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/project/ucos_iii/src/subdir.mk
-include src/library/middlewares/uart/subdir.mk
-include src/library/middlewares/spi_flash/subdir.mk
-include src/library/middlewares/psram/subdir.mk
-include src/library/middlewares/hyper_ram/subdir.mk
-include src/library/middlewares/gpio/subdir.mk
-include src/library/middlewares/dmm/subdir.mk
-include src/library/middlewares/delay/subdir.mk
-include src/library/middlewares/3rd_party/ucos_iii/OS/Source/subdir.mk
-include src/library/middlewares/3rd_party/ucos_iii/OS/Ports/cm3/Generic/GMD/subdir.mk
-include src/library/middlewares/3rd_party/ucos_iii/LIB/Ports/cm3/GMD/subdir.mk
-include src/library/middlewares/3rd_party/ucos_iii/LIB/subdir.mk
-include src/library/middlewares/3rd_party/ucos_iii/CPU/cm3/GMD/subdir.mk
-include src/library/middlewares/3rd_party/ucos_iii/CPU/subdir.mk
-include src/library/middlewares/3rd_party/ucos_iii/CONFIG/subdir.mk
-include src/library/middlewares/3rd_party/ucos_iii/BSP/subdir.mk
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
cm3_ucos_iii.bin \

SECONDARY_SIZE += \
cm3_ucos_iii.siz \


# All Target
all: cm3_ucos_iii.elf secondary-outputs

# Tool invocations
cm3_ucos_iii.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM Cross C Linker'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -T "D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\libraries\cmsis\cm3\device_support\startup\gmd\linker\gw1ns4c_flash.ld" -Xlinker --gc-sections -Wl,-Map,"cm3_ucos_iii.map" -o "cm3_ucos_iii.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

cm3_ucos_iii.bin: cm3_ucos_iii.elf
	@echo 'Invoking: GNU ARM Cross Create Flash Image'
	arm-none-eabi-objcopy -O binary "cm3_ucos_iii.elf"  "cm3_ucos_iii.bin"
	@echo 'Finished building: $@'
	@echo ' '

cm3_ucos_iii.siz: cm3_ucos_iii.elf
	@echo 'Invoking: GNU ARM Cross Print Size'
	arm-none-eabi-size --format=berkeley "cm3_ucos_iii.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) cm3_ucos_iii.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
