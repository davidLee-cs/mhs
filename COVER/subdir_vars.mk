################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../2837xD_FLASH_lnk_cpu1.cmd 

SYSCFG_SRCS += \
../ARINC429.syscfg 

LIB_SRCS += \
C:/ti/c2000/C2000Ware_5_01_00_00/driverlib/f2837xd/driverlib/ccs/Debug/driverlib.lib 

C_SRCS += \
./syscfg/board.c \
./syscfg/c2000ware_libraries.c \
../HI_3587_arinc.c \
../csu_Cbit.c \
../csu_accelMeasurement.c \
../csu_angle_calculation.c \
../csu_calibrationData.c \
../csu_calibration_mode.c \
../csu_factory_mode.c \
../csu_fluxMeasurement.c \
../csu_init.c \
../csu_mhs_Status_trans.c \
../csu_operation_mode.c \
../csu_status_mode.c \
../csu_tempMeasurement.c \
../csu_test_status_trans.c \
../csu_voltageMeasurement.c \
../i2cLib_FIFO_master_interrupt.c \
../i2c_ex6_eeprom_interrupt.c \
../lib_mhsAdc.c \
../lib_mhsEpwm.c \
../lib_mhsI2c.c \
../lib_mhsStd.c \
../lib_mhsUart.c \
../main.c 

GEN_FILES += \
./syscfg/board.c \
./syscfg/board.opt \
./syscfg/c2000ware_libraries.opt \
./syscfg/c2000ware_libraries.c 

GEN_MISC_DIRS += \
./syscfg/ 

C_DEPS += \
./syscfg/board.d \
./syscfg/c2000ware_libraries.d \
./HI_3587_arinc.d \
./csu_Cbit.d \
./csu_accelMeasurement.d \
./csu_angle_calculation.d \
./csu_calibrationData.d \
./csu_calibration_mode.d \
./csu_factory_mode.d \
./csu_fluxMeasurement.d \
./csu_init.d \
./csu_mhs_Status_trans.d \
./csu_operation_mode.d \
./csu_status_mode.d \
./csu_tempMeasurement.d \
./csu_test_status_trans.d \
./csu_voltageMeasurement.d \
./i2cLib_FIFO_master_interrupt.d \
./i2c_ex6_eeprom_interrupt.d \
./lib_mhsAdc.d \
./lib_mhsEpwm.d \
./lib_mhsI2c.d \
./lib_mhsStd.d \
./lib_mhsUart.d \
./main.d 

GEN_OPTS += \
./syscfg/board.opt \
./syscfg/c2000ware_libraries.opt 

OBJS += \
./syscfg/board.obj \
./syscfg/c2000ware_libraries.obj \
./HI_3587_arinc.obj \
./csu_Cbit.obj \
./csu_accelMeasurement.obj \
./csu_angle_calculation.obj \
./csu_calibrationData.obj \
./csu_calibration_mode.obj \
./csu_factory_mode.obj \
./csu_fluxMeasurement.obj \
./csu_init.obj \
./csu_mhs_Status_trans.obj \
./csu_operation_mode.obj \
./csu_status_mode.obj \
./csu_tempMeasurement.obj \
./csu_test_status_trans.obj \
./csu_voltageMeasurement.obj \
./i2cLib_FIFO_master_interrupt.obj \
./i2c_ex6_eeprom_interrupt.obj \
./lib_mhsAdc.obj \
./lib_mhsEpwm.obj \
./lib_mhsI2c.obj \
./lib_mhsStd.obj \
./lib_mhsUart.obj \
./main.obj 

GEN_MISC_FILES += \
./syscfg/board.h \
./syscfg/board.cmd.genlibs \
./syscfg/pinmux.csv \
./syscfg/c2000ware_libraries.cmd.genlibs \
./syscfg/c2000ware_libraries.h \
./syscfg/clocktree.h 

GEN_MISC_DIRS__QUOTED += \
"syscfg\" 

OBJS__QUOTED += \
"syscfg\board.obj" \
"syscfg\c2000ware_libraries.obj" \
"HI_3587_arinc.obj" \
"csu_Cbit.obj" \
"csu_accelMeasurement.obj" \
"csu_angle_calculation.obj" \
"csu_calibrationData.obj" \
"csu_calibration_mode.obj" \
"csu_factory_mode.obj" \
"csu_fluxMeasurement.obj" \
"csu_init.obj" \
"csu_mhs_Status_trans.obj" \
"csu_operation_mode.obj" \
"csu_status_mode.obj" \
"csu_tempMeasurement.obj" \
"csu_test_status_trans.obj" \
"csu_voltageMeasurement.obj" \
"i2cLib_FIFO_master_interrupt.obj" \
"i2c_ex6_eeprom_interrupt.obj" \
"lib_mhsAdc.obj" \
"lib_mhsEpwm.obj" \
"lib_mhsI2c.obj" \
"lib_mhsStd.obj" \
"lib_mhsUart.obj" \
"main.obj" 

GEN_MISC_FILES__QUOTED += \
"syscfg\board.h" \
"syscfg\board.cmd.genlibs" \
"syscfg\pinmux.csv" \
"syscfg\c2000ware_libraries.cmd.genlibs" \
"syscfg\c2000ware_libraries.h" \
"syscfg\clocktree.h" 

C_DEPS__QUOTED += \
"syscfg\board.d" \
"syscfg\c2000ware_libraries.d" \
"HI_3587_arinc.d" \
"csu_Cbit.d" \
"csu_accelMeasurement.d" \
"csu_angle_calculation.d" \
"csu_calibrationData.d" \
"csu_calibration_mode.d" \
"csu_factory_mode.d" \
"csu_fluxMeasurement.d" \
"csu_init.d" \
"csu_mhs_Status_trans.d" \
"csu_operation_mode.d" \
"csu_status_mode.d" \
"csu_tempMeasurement.d" \
"csu_test_status_trans.d" \
"csu_voltageMeasurement.d" \
"i2cLib_FIFO_master_interrupt.d" \
"i2c_ex6_eeprom_interrupt.d" \
"lib_mhsAdc.d" \
"lib_mhsEpwm.d" \
"lib_mhsI2c.d" \
"lib_mhsStd.d" \
"lib_mhsUart.d" \
"main.d" 

GEN_FILES__QUOTED += \
"syscfg\board.c" \
"syscfg\board.opt" \
"syscfg\c2000ware_libraries.opt" \
"syscfg\c2000ware_libraries.c" 

SYSCFG_SRCS__QUOTED += \
"../ARINC429.syscfg" 

C_SRCS__QUOTED += \
"./syscfg/board.c" \
"./syscfg/c2000ware_libraries.c" \
"../HI_3587_arinc.c" \
"../csu_Cbit.c" \
"../csu_accelMeasurement.c" \
"../csu_angle_calculation.c" \
"../csu_calibrationData.c" \
"../csu_calibration_mode.c" \
"../csu_factory_mode.c" \
"../csu_fluxMeasurement.c" \
"../csu_init.c" \
"../csu_mhs_Status_trans.c" \
"../csu_operation_mode.c" \
"../csu_status_mode.c" \
"../csu_tempMeasurement.c" \
"../csu_test_status_trans.c" \
"../csu_voltageMeasurement.c" \
"../i2cLib_FIFO_master_interrupt.c" \
"../i2c_ex6_eeprom_interrupt.c" \
"../lib_mhsAdc.c" \
"../lib_mhsEpwm.c" \
"../lib_mhsI2c.c" \
"../lib_mhsStd.c" \
"../lib_mhsUart.c" \
"../main.c" 


