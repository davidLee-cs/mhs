################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS

GEN_OPTS__FLAG := --cmd_file="syscfg/board.opt" --cmd_file="syscfg/c2000ware_libraries.opt" 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./syscfg/board.obj" \
"./syscfg/c2000ware_libraries.obj" \
"./HI_3587_arinc.obj" \
"./csu_Cbit.obj" \
"./csu_accelMeasurement.obj" \
"./csu_angle_calculation.obj" \
"./csu_calibrationData.obj" \
"./csu_calibration_mode.obj" \
"./csu_factory_mode.obj" \
"./csu_fluxMeasurement.obj" \
"./csu_init.obj" \
"./csu_mhs_Status_trans.obj" \
"./csu_operation_mode.obj" \
"./csu_status_mode.obj" \
"./csu_tempMeasurement.obj" \
"./csu_test_status_trans.obj" \
"./csu_voltageMeasurement.obj" \
"./i2cLib_FIFO_master_interrupt.obj" \
"./i2c_ex6_eeprom_interrupt.obj" \
"./lib_mhsAdc.obj" \
"./lib_mhsEpwm.obj" \
"./lib_mhsI2c.obj" \
"./lib_mhsStd.obj" \
"./lib_mhsUart.obj" \
"./main.obj" \
"./device/F2837xD_CodeStartBranch.obj" \
"./device/device.obj" \
"../2837xD_FLASH_lnk_cpu1.cmd" \
"C:/ti/c2000/C2000Ware_5_01_00_00/driverlib/f2837xd/driverlib/ccs/Debug/driverlib.lib" \
$(GEN_CMDS__FLAG) \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include device/subdir_vars.mk
-include subdir_rules.mk
-include device/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
KF21_HMS_Arinc429_ver1p5.out \

EXE_OUTPUTS__QUOTED += \
"KF21_HMS_Arinc429_ver1p5.out" \

BIN_OUTPUTS += \
KF21_HMS_Arinc429_ver1p5.hex \

BIN_OUTPUTS__QUOTED += \
"KF21_HMS_Arinc429_ver1p5.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "KF21_HMS_Arinc429_ver1p5.out"

# Tool invocations
KF21_HMS_Arinc429_ver1p5.out: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: C2000 Linker'
	"C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -Ooff --advice:performance=all --define=_LAUNCHXL_F28379D --define=DEBUG --define=CPU1 --define=_FLASH --c99 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi -z -m"KF21_HMS_Arinc429_ver1p5.map" --heap_size=0x200 --stack_size=0x3F8 --warn_sections -i"C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/lib" -i"C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --reread_libs --diag_wrap=off --display_error_number --xml_link_info="KF21_HMS_Arinc429_ver1p5_linkInfo.xml" --entry_point=code_start --rom_model -o "KF21_HMS_Arinc429_ver1p5.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

KF21_HMS_Arinc429_ver1p5.hex: $(EXE_OUTPUTS)
	@echo 'Building secondary target: "$@"'
	@echo 'Invoking: C2000 Hex Utility'
	"C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/hex2000" --diag_wrap=off -o "KF21_HMS_Arinc429_ver1p5.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_MISC_FILES__QUOTED)$(BIN_OUTPUTS__QUOTED)$(GEN_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "syscfg\board.obj" "syscfg\c2000ware_libraries.obj" "HI_3587_arinc.obj" "csu_Cbit.obj" "csu_accelMeasurement.obj" "csu_angle_calculation.obj" "csu_calibrationData.obj" "csu_calibration_mode.obj" "csu_factory_mode.obj" "csu_fluxMeasurement.obj" "csu_init.obj" "csu_mhs_Status_trans.obj" "csu_operation_mode.obj" "csu_status_mode.obj" "csu_tempMeasurement.obj" "csu_test_status_trans.obj" "csu_voltageMeasurement.obj" "i2cLib_FIFO_master_interrupt.obj" "i2c_ex6_eeprom_interrupt.obj" "lib_mhsAdc.obj" "lib_mhsEpwm.obj" "lib_mhsI2c.obj" "lib_mhsStd.obj" "lib_mhsUart.obj" "main.obj" "device\F2837xD_CodeStartBranch.obj" "device\device.obj" 
	-$(RM) "syscfg\board.d" "syscfg\c2000ware_libraries.d" "HI_3587_arinc.d" "csu_Cbit.d" "csu_accelMeasurement.d" "csu_angle_calculation.d" "csu_calibrationData.d" "csu_calibration_mode.d" "csu_factory_mode.d" "csu_fluxMeasurement.d" "csu_init.d" "csu_mhs_Status_trans.d" "csu_operation_mode.d" "csu_status_mode.d" "csu_tempMeasurement.d" "csu_test_status_trans.d" "csu_voltageMeasurement.d" "i2cLib_FIFO_master_interrupt.d" "i2c_ex6_eeprom_interrupt.d" "lib_mhsAdc.d" "lib_mhsEpwm.d" "lib_mhsI2c.d" "lib_mhsStd.d" "lib_mhsUart.d" "main.d" "device\device.d" 
	-$(RM) "device\F2837xD_CodeStartBranch.d" 
	-$(RMDIR) $(GEN_MISC_DIRS__QUOTED)
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

