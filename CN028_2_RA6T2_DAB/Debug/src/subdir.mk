################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/can_fd.c \
../src/console.c \
../src/dcdc_dab.c \
../src/err_handler.c \
../src/hal_entry.c \
../src/ra6t2.c 

C_DEPS += \
./src/can_fd.d \
./src/console.d \
./src/dcdc_dab.d \
./src/err_handler.d \
./src/hal_entry.d \
./src/ra6t2.d 

OBJS += \
./src/can_fd.o \
./src/console.o \
./src/dcdc_dab.o \
./src/err_handler.o \
./src/hal_entry.o \
./src/ra6t2.o 

SREC += \
CN028_2_RA6T2_DAB.srec 

MAP += \
CN028_2_RA6T2_DAB.map 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Ofast -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fsingle-precision-constant -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -gdwarf-4 -D_RA_CORE=CM33 -D_RENESAS_RA_ -I"D:/Workspace/gitLab_workspace/cn028-2-digital-power-dab/CN028_2_RA6T2_DAB/src" -I"D:/Workspace/gitLab_workspace/cn028-2-digital-power-dab/CN028_2_RA6T2_DAB/lib" -I"D:/Workspace/gitLab_workspace/cn028-2-digital-power-dab/CN028_2_RA6T2_DAB/src" -I"D:/Workspace/gitLab_workspace/cn028-2-digital-power-dab/CN028_2_RA6T2_DAB/ra/fsp/inc" -I"D:/Workspace/gitLab_workspace/cn028-2-digital-power-dab/CN028_2_RA6T2_DAB/ra/fsp/inc/api" -I"D:/Workspace/gitLab_workspace/cn028-2-digital-power-dab/CN028_2_RA6T2_DAB/ra/fsp/inc/instances" -I"D:/Workspace/gitLab_workspace/cn028-2-digital-power-dab/CN028_2_RA6T2_DAB/ra/arm/CMSIS_5/CMSIS/Core/Include" -I"D:/Workspace/gitLab_workspace/cn028-2-digital-power-dab/CN028_2_RA6T2_DAB/ra_gen" -I"D:/Workspace/gitLab_workspace/cn028-2-digital-power-dab/CN028_2_RA6T2_DAB/ra_cfg/fsp_cfg/bsp" -I"D:/Workspace/gitLab_workspace/cn028-2-digital-power-dab/CN028_2_RA6T2_DAB/ra_cfg/fsp_cfg" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

