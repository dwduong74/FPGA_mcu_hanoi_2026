################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/library/middlewares/3rd_party/rtthread_nano/src/clock.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/components.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/cpu.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/idle.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/ipc.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/irq.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/kservice.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/mem.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/memheap.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/mempool.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/object.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/scheduler.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/slab.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/thread.c \
../src/library/middlewares/3rd_party/rtthread_nano/src/timer.c 

OBJS += \
./src/library/middlewares/3rd_party/rtthread_nano/src/clock.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/components.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/cpu.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/idle.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/ipc.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/irq.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/kservice.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/mem.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/memheap.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/mempool.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/object.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/scheduler.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/slab.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/thread.o \
./src/library/middlewares/3rd_party/rtthread_nano/src/timer.o 

C_DEPS += \
./src/library/middlewares/3rd_party/rtthread_nano/src/clock.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/components.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/cpu.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/idle.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/ipc.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/irq.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/kservice.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/mem.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/memheap.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/mempool.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/object.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/scheduler.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/slab.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/thread.d \
./src/library/middlewares/3rd_party/rtthread_nano/src/timer.d 


# Each subdirectory must supply rules for building sources it contributes
src/library/middlewares/3rd_party/rtthread_nano/src/%.o: ../src/library/middlewares/3rd_party/rtthread_nano/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\libraries\cmsis\cm3\core_support\gmd" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\libraries\cmsis\cm3\device_support" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\libraries\drivers\inc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\gpio" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\uart" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\3rd_party\rtthread_nano\bsp\cm3" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\3rd_party\rtthread_nano\components\finsh" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\3rd_party\rtthread_nano\include" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\library\middlewares\3rd_party\rtthread_nano\include\libc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_rtthread_nano\src\project\rtthread_nano\inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


