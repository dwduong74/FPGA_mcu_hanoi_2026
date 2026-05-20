################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../src/library/libraries/cmsis/cm3/device_support/startup/gmd/startup_gw1ns4c.S 

OBJS += \
./src/library/libraries/cmsis/cm3/device_support/startup/gmd/startup_gw1ns4c.o 

S_UPPER_DEPS += \
./src/library/libraries/cmsis/cm3/device_support/startup/gmd/startup_gw1ns4c.d 


# Each subdirectory must supply rules for building sources it contributes
src/library/libraries/cmsis/cm3/device_support/startup/gmd/%.o: ../src/library/libraries/cmsis/cm3/device_support/startup/gmd/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -x assembler-with-cpp -D__STARTUP_CLEAR_BSS -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


