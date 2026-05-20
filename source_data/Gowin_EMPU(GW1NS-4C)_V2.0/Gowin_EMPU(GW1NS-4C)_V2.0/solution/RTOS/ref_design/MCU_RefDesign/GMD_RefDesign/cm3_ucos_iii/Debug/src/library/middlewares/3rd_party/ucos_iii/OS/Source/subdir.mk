################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_cfg_app.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_core.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_dbg.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_flag.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_int.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_mem.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_msg.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_mutex.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_pend_multi.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_prio.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_q.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_sem.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_stat.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_task.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_tick.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_time.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_tmr.c \
../src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_var.c 

OBJS += \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_cfg_app.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_core.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_dbg.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_flag.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_int.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_mem.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_msg.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_mutex.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_pend_multi.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_prio.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_q.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_sem.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_stat.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_task.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_tick.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_time.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_tmr.o \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_var.o 

C_DEPS += \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_cfg_app.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_core.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_dbg.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_flag.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_int.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_mem.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_msg.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_mutex.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_pend_multi.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_prio.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_q.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_sem.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_stat.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_task.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_tick.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_time.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_tmr.d \
./src/library/middlewares/3rd_party/ucos_iii/OS/Source/os_var.d 


# Each subdirectory must supply rules for building sources it contributes
src/library/middlewares/3rd_party/ucos_iii/OS/Source/%.o: ../src/library/middlewares/3rd_party/ucos_iii/OS/Source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\libraries\cmsis\cm3\core_support\gmd" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\project\ucos_iii\inc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\libraries\cmsis\cm3\device_support" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\libraries\drivers\inc" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\gpio" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\uart" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\BSP" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\CONFIG" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\CPU" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\CPU\cm3\GMD" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\LIB" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\OS\Source" -I"D:\GMD_workspace\workspace_cm3_hard\cm3_ucos_iii\src\library\middlewares\3rd_party\ucos_iii\OS\Ports\cm3\Generic\GMD" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


