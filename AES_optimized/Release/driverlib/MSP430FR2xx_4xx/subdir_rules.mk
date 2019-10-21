################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
driverlib/MSP430FR2xx_4xx/%.obj: ../driverlib/MSP430FR2xx_4xx/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/home/simon/ti/ccs920/ccs/tools/compiler/ti-cgt-msp430_18.12.3.LTS/bin/cl430" -vmspx -O0 --use_hw_mpy=none --include_path="/home/simon/workspace_v9/AES256_MSP430/AES_optimized/driverlib/MSP430FR2xx_4xx" --include_path="/home/simon/ti/ccs920/ccs/ccs_base/msp430/include" --include_path="/home/simon/workspace_v9/AES256_MSP430/AES_optimized" --include_path="/home/simon/ti/ccs920/ccs/tools/compiler/ti-cgt-msp430_18.12.3.LTS/include" --advice:power=all --advice:hw_config="all" --define=__MSP430FR4133__ --printf_support=nofloat --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="driverlib/MSP430FR2xx_4xx/$(basename $(<F)).d_raw" --obj_directory="driverlib/MSP430FR2xx_4xx" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


