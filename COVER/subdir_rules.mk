################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-2027435105: ../ARINC429.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/sysconfig_1_16_2/sysconfig_cli.bat" -s "C:/ti/c2000/C2000Ware_5_01_00_00/.metadata/sdk.json" -d "F2837xD" -p "F2837xD_176PTP" -r "F2837xD_176PTP" --script "C:/myProject/SensorPia_Project/HMS/KF21_HMS_Arinc429_ver1p5/KF21_HMS_Arinc429_ver1p5/ARINC429.syscfg" -o "syscfg" --compiler ccs
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/board.c: build-2027435105 ../ARINC429.syscfg
syscfg/board.h: build-2027435105
syscfg/board.cmd.genlibs: build-2027435105
syscfg/board.opt: build-2027435105
syscfg/pinmux.csv: build-2027435105
syscfg/c2000ware_libraries.cmd.genlibs: build-2027435105
syscfg/c2000ware_libraries.opt: build-2027435105
syscfg/c2000ware_libraries.c: build-2027435105
syscfg/c2000ware_libraries.h: build-2027435105
syscfg/clocktree.h: build-2027435105
syscfg/: build-2027435105

syscfg/%.obj: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	C:\QualityScroll-Cover-SE\wrapper\kf21\cl2000.exe -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -Ooff --include_path="C:/myProject/SensorPia_Project/HMS/KF21_HMS_Arinc429_ver1p5/KF21_HMS_Arinc429_ver1p5" --include_path="C:/myProject/SensorPia_Project/HMS/KF21_HMS_Arinc429_ver1p5/KF21_HMS_Arinc429_ver1p5/device" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/driverlib/f2837xd/driverlib" --include_path="C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --include_path="C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include/sys" --advice:performance=all --define=_LAUNCHXL_F28379D --define=DEBUG --define=CPU1 --define=_FLASH --c99 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="syscfg/$(basename $(<F)).d_raw" --include_path="C:/myProject/SensorPia_Project/HMS/KF21_HMS_Arinc429_ver1p5/KF21_HMS_Arinc429_ver1p5/COVER/syscfg" --obj_directory="syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	C:\QualityScroll-Cover-SE\wrapper\kf21\cl2000.exe -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -Ooff --include_path="C:/myProject/SensorPia_Project/HMS/KF21_HMS_Arinc429_ver1p5/KF21_HMS_Arinc429_ver1p5" --include_path="C:/myProject/SensorPia_Project/HMS/KF21_HMS_Arinc429_ver1p5/KF21_HMS_Arinc429_ver1p5/device" --include_path="C:/ti/c2000/C2000Ware_5_01_00_00/driverlib/f2837xd/driverlib" --include_path="C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --include_path="C:/ti/ccs1120/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include/sys" --advice:performance=all --define=_LAUNCHXL_F28379D --define=DEBUG --define=CPU1 --define=_FLASH --c99 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" --include_path="C:/myProject/SensorPia_Project/HMS/KF21_HMS_Arinc429_ver1p5/KF21_HMS_Arinc429_ver1p5/COVER/syscfg" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


