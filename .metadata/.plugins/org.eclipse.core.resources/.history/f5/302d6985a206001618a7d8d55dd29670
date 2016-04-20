################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := D:/ti/ccsv5/tools/compiler/c6000_7.4.4

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./common/common_test.obj" \
"./common/Keystone_Serdes_init.obj" \
"./common/Keystone_I2C_init_drv.obj" \
"./common/KeyStone_common.obj" \
"./common/KeyStone_UART_Init_drv.obj" \
"./common/KeyStone_SRIO_Init_drv.obj" \
"./common/KeyStone_SPI_Init_drv.obj" \
"./common/KeyStone_PCIE_Init_drv.obj" \
"./common/KeyStone_Navigator_init_drv.obj" \
"./common/KeyStone_HyperLink_Init.obj" \
"./common/KeyStone_GE_Init_drv.obj" \
"./common/KeyStone_EMIF16_Init.obj" \
"./common/KeyStone_DDR_Init.obj" \
"./common/INT_vectors.obj" \
"./common/CPU_access_test.obj" \
"./common/CPU_LD_ST_Test.obj" \
"./srio_drv.obj" \
"./qmss_device.obj" \
"./main1.obj" \
"./loopbackDioIsr_osal.obj" \
"./device_srio_loopback.obj" \
"./cppi_drv.obj" \
"./cppi_device.obj" \
"./SRIO_switch.obj" \
-l"libc.a" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include common/subdir_vars.mk
-include subdir_rules.mk
-include common/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: simulator_vpx_DSP1_V0_1_6_core1.out

# Tool invocations
simulator_vpx_DSP1_V0_1_6_core1.out: $(OBJS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: C6000 Linker'
	"D:/ti/ccsv5/tools/compiler/c6000_7.4.4/bin/cl6x" -mv6600 --abi=eabi -g --display_error_number --diag_warning=225 --diag_wrap=off -z -m"simulator_vpx_DSP1_V0_1_6_core1.map" -i"D:/ti/ccsv5/tools/compiler/c6000_7.4.4/lib" -i"D:/ti/ccsv5/tools/compiler/c6000_7.4.4/include" --reread_libs --warn_sections --display_error_number --diag_wrap=off --xml_link_info="simulator_vpx_DSP1_V0_1_6_core1_linkInfo.xml" --rom_model -o "simulator_vpx_DSP1_V0_1_6_core1.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_SRCS__QUOTED)$(GEN_MISC_FILES__QUOTED)$(C6000_EXECUTABLE_OUTPUTS__QUOTED)$(GEN_OPTS__QUOTED)$(GEN_CMDS__QUOTED) "simulator_vpx_DSP1_V0_1_6_core1.out"
	-$(RMDIR) $(GEN_MISC_DIRS__QUOTED)
	-$(RM) "SRIO_switch.pp" "cppi_device.pp" "cppi_drv.pp" "device_srio_loopback.pp" "loopbackDioIsr_osal.pp" "main1.pp" "qmss_device.pp" "srio_drv.pp" "common\CPU_access_test.pp" "common\KeyStone_DDR_Init.pp" "common\KeyStone_EMIF16_Init.pp" "common\KeyStone_GE_Init_drv.pp" "common\KeyStone_HyperLink_Init.pp" "common\KeyStone_Navigator_init_drv.pp" "common\KeyStone_PCIE_Init_drv.pp" "common\KeyStone_SPI_Init_drv.pp" "common\KeyStone_SRIO_Init_drv.pp" "common\KeyStone_UART_Init_drv.pp" "common\KeyStone_common.pp" "common\Keystone_I2C_init_drv.pp" "common\Keystone_Serdes_init.pp" "common\common_test.pp" 
	-$(RM) "SRIO_switch.obj" "cppi_device.obj" "cppi_drv.obj" "device_srio_loopback.obj" "loopbackDioIsr_osal.obj" "main1.obj" "qmss_device.obj" "srio_drv.obj" "common\CPU_LD_ST_Test.obj" "common\CPU_access_test.obj" "common\INT_vectors.obj" "common\KeyStone_DDR_Init.obj" "common\KeyStone_EMIF16_Init.obj" "common\KeyStone_GE_Init_drv.obj" "common\KeyStone_HyperLink_Init.obj" "common\KeyStone_Navigator_init_drv.obj" "common\KeyStone_PCIE_Init_drv.obj" "common\KeyStone_SPI_Init_drv.obj" "common\KeyStone_SRIO_Init_drv.obj" "common\KeyStone_UART_Init_drv.obj" "common\KeyStone_common.obj" "common\Keystone_I2C_init_drv.obj" "common\Keystone_Serdes_init.obj" "common\common_test.obj" 
	-$(RM) "common\CPU_LD_ST_Test.pp" "common\INT_vectors.pp" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
