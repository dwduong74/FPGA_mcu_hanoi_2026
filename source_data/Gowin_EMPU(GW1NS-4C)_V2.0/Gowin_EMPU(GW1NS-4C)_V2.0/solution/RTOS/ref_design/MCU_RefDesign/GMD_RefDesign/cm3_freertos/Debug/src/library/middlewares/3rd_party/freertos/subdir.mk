################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/library/middlewares/3rd_party/freertos/croutine.c \
../src/library/middlewares/3rd_party/freertos/event_groups.c \
../src/library/middlewares/3rd_party/freertos/list.c \
../src/library/middlewares/3rd_party/freertos/queue.c \
../src/library/middlewares/3rd_party/freertos/stream_buffer.c \
../src/library/middlewares/3rd_party/freertos/tasks.c \
../src/library/middlewares/3rd_party/freertos/timers.c 

OBJS += \
./src/library/middlewares/3rd_party/freertos/croutine.o \
./src/library/middlewares/3rd_party/freertos/event_groups.o \
./src/library/middlewares/3rd_party/freertos/list.o \
./src/library/middlewares/3rd_party/freertos/queue.o \
./src/library/middlewares/3rd_party/freertos/stream_buffer.o \
./src/library/middlewares/3rd_party/freertos/tasks.o \
./src/library/middlewares/3rd_party/freertos/timers.o 

C_DEPS += \
./src/library/middlewares/3rd_party/freertos/croutine.d \
./src/library/middlewares/3rd_party/freertos/event_groups.d \
./src/library/middlewares/3rd_party/freertos/list.d \
./src/library/middlewares/3rd_party/freertos/queue.d \
./src/library/middlewares/3rd_party/freertos/stream_buffer.d \
./src/library/middlewares/3rd_party/freertos/tasks.d \
./src/library/middlewares/3rd_party/freertos/timers.d 


# Each subdirectory must supply rules for building sources it contributes
src/library/middlewares/3rd_party/freertos/%.o: ../src/library/middlewares/3rd_party/freertos/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\libraries\cmsis\cm3\core_support\gmd" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\libraries\cmsis\cm3\device_support" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\libraries\drivers\inc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\middlewares\gpio" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\middlewares\uart" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\middlewares\3rd_party\freertos\include" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\library\middlewares\3rd_party\freertos\portable\GMD\cm3" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_freertos\src\project\freertos\inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


