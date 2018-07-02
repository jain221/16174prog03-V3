#
# This is the root makefile for the 16174prog03 project.
#
-include source/unit_test_local_config.mke

ifeq ($(UNIT_TEST_IGNORE_ERRORS),-)
  ${info WARNING Failed Unit-Tests are being ignored!}
endif


#TARGET += build_test
TARGET += build_src


all: $(TARGET)

clean: clean_test clean_src


# The ST Cube software generates the 'list.c' file as part of the FreeRTOS library.
# However is clashes with the 'list.c' file in the Contiki-OS library.
# Here we rename the 'list.c' file in the FreeRTOS library to 'freertos_list.c'
../contiki-alc/platform/16174a03-gateway/Middlewares/Third_Party/FreeRTOS/Source/freertos_list.c:
	mv ../contiki-alc/platform/16174a03-gateway/Middlewares/Third_Party/FreeRTOS/Source/list.c ../contiki-alc/platform/16174a03-gateway/Middlewares/Third_Party/FreeRTOS/Source/freertos_list.c


build_src: ../contiki-alc/platform/16174a03-gateway/Middlewares/Third_Party/FreeRTOS/Source/freertos_list.c
	@echo "################################################################"
	@echo "################## BUILDING SOURCE #############################"
	@echo "################################################################"
	$(MAKE)	-C source all
	@echo
	@echo

clean_src:
	-$(MAKE) -C source clean


build_test:
	@echo "################################################################"
	@echo "################### RUNNING TESTS ##############################"
	@echo "################################################################"
	$(UNIT_TEST_IGNORE_ERRORS)$(MAKE) -C source -f unit_test.mke all
	@echo
	@echo

clean_test:
	-$(MAKE) -C source -f unit_test.mke clean
