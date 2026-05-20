################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../src/library/middlewares/3rd_party/ucos_iii/LIB/Ports/cm3/GMD/lib_mem_a.S 

OBJS += \
./src/library/middlewares/3rd_party/ucos_iii/LIB/Ports/cm3/GMD/lib_mem_a.o 

S_UPPER_DEPS += \
./src/library/middlewares/3rd_party/ucos_iii/LIB/Ports/cm3/GMD/lib_mem_a.d 


# Each subdirectory must supply rules for building sources it contributes
src/library/middlewares/3rd_party/ucos_iii/LIB/Ports/cm3/GMD/%.o: ../src/library/middlewares/3rd_party/ucos_iii/LIB/Ports/cm3/GMD/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -x assembler-with-cpp -D__STARTUP_CLEAR_BSS -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


