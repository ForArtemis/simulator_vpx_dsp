################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Calculate.obj: ../Calculate.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"D:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="D:/ti/pdk_C6678_1_1_2_6/packages/ti/drv/srio/example/SRIOLoopbackDioIsr" --include_path="D:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="D:/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="D:/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="D:/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --diag_warning=225 --preproc_with_compile --preproc_dependency="Calculate.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/compiler.opt: ../core2.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"D:/ti/xdctools_3_25_03_72/xs" --xdcpath="D:/ti/pdk_C6678_1_1_2_6/packages;D:/ti/bios_6_35_04_50/packages;D:/ti/mcsdk_2_01_02_06/demos;D:/ti/ccsv5/ccs_base;D:/ti/ipc_1_24_03_32/packages;" xdc.tools.configuro -o configPkg -t ti.targets.elf.C66 -p ti.platforms.evm6678 -r debug -c "D:/ti/ccsv5/tools/compiler/c6000_7.4.4" "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/linker.cmd: configPkg/compiler.opt
configPkg/: configPkg/compiler.opt

main2.obj: ../main2.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"D:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --include_path="D:/ti/pdk_C6678_1_1_2_6/packages/ti/drv/srio/example/SRIOLoopbackDioIsr" --include_path="D:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --include_path="D:/ti/pdk_C6678_1_1_2_6/packages/ti/csl" --include_path="D:/ti/pdk_C6678_1_1_2_6/packages/ti/drv/qmss" --include_path="D:/ti/pdk_C6678_1_1_2_6/packages/ti/drv/cppi" --diag_warning=225 --preproc_with_compile --preproc_dependency="main2.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


