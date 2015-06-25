#DEBUG:=0

ARCH:=$(shell uname -m)

ifneq ($(ARCH),mips)
ifeq ($(TARGET),mipsel)
#ARCH is not mips but cross-compiling for mips on host machine
export STAGING_DIR:=/opt/mips_toolchain
CROSSTOOL:=$(STAGING_DIR)/bin
export CC:=$(CROSSTOOL)/mips-linux-gnu-gcc -EL
else
ifeq ($(TARGET),)
#ARCH is not mips ,and host & target are same i.e. linux PC
TARGET:=$(ARCH)
endif
endif
else
#ARCH is mips and host & target are same i.e. mips board
TARGET:=mipsel
endif

ifeq ($(TARGET),$(ARCH))
PYTHON_VERSION:=$(shell python --version 2>&1 | gawk '{ print substr($$2,0,3) }')
export PYTHON_INC:=-I/usr/include/python$(PYTHON_VERSION)
endif

export DIR__DEBUG:=$(if $(DEBUG),debug,release)
export DIR__OBJ:=$(DIR__BUILD)/$(TARGET)/$(DIR__DEBUG)
export DIR__STAGING:=/tmp/$(TARGET)/$(DIR__DEBUG)/usr

$(DIR__OBJ) \
$(DIR__STAGING)/lib:
	mkdir -p $@


space :=
space +=

info:
	@echo DIR__OBJ $(DIR__OBJ)
	@echo DIR__SRC $(DIR__SRC)
	@echo DIR__SDK $(DIR__SDK) = $(shell cd $(DIR__SDK) ; pwd)
	@echo -e SRC \\n\\t$(subst $(space),\\n\\t,$(SRC))
	@echo -e C_OBJ \\n\\t$(subst $(space),\\n\\t,$(C_OBJ))
