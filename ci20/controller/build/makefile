all: bin

DIR__BUILD:=$(PWD)

include rules.mk

DIR__RELEASE:=$(DIR__BUILD)/../release
DIR__CONTROLLER_RELEASE:=$(DIR__RELEASE)/flowclimatecontroller
DIR__BIN:=$(DIR__BUILD)/../bin
DIR__SRC:=$(DIR__BUILD)/../src
BINARY:=$(DIR__BIN)/flowclimatecontroller.$(TARGET).bin
bin: $(BINARY)

RM := rm -rf

VERSION_MAJOR=$(shell cat version | cut -d \. -f 1)
VERSION_MINOR=$(shell cat version | cut -d \. -f 2)
VERSION_REVSION=$(shell cat version | cut -d \. -f 3)

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables

# Tool invocations
$(BINARY): $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	mkdir -p $(dir $@)
	$(CC)  -o "$@" $? $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS) $(C_DEP) $(LIBRARIES) $(DIR__BIN) $(DIR__BUILD)/$(TARGET) $(DIR__RELEASE)
	-@echo ' '

archive:
	mkdir -p $(DIR__CONTROLLER_RELEASE)
	$(RM) $(DIR__CONTROLLER_RELEASE)/*
	cp version $(DIR__RELEASE)
	cp $(BINARY) $(DIR__RELEASE)
	tar -C $(DIR__RELEASE) -czf $(DIR__RELEASE)/flowclimatecontroller.${TARGET}.tar.gz version flowclimatecontroller.$(TARGET).bin
	cp -r $(DIR__BUILD) $(DIR__CONTROLLER_RELEASE)
	cp -r $(DIR__SRC) $(DIR__CONTROLLER_RELEASE)
	cp -r $(DIR__BIN) $(DIR__CONTROLLER_RELEASE)
	$(RM) $(DIR__CONTROLLER_RELEASE)/build/$(TARGET)
	chmod -R +rw $(DIR__CONTROLLER_RELEASE)
	tar -C $(DIR__RELEASE) -czf $(DIR__RELEASE)/flowclimatecontroller.$(TARGET).src.tar.gz flowclimatecontroller/
