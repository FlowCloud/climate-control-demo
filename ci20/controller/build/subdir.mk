DIR__SRC=../src
SRC=$(addprefix $(DIR__SRC)/,	\
	./main.c \
	./controller.c \
	./flow_interface.c \
	./flow_interface_func.c \
	./console.c \
	./construct_message.c \
	./controller_logging.c \
)

DIR__LIB:=../

C_SRC:=$(filter %.c,$(SRC))
C_OBJ:=$(patsubst $(DIR__SRC)/%.c,$(DIR__OBJ)/%.o,$(C_SRC))
C_DEP:=$(C_OBJ:.o=.d)

OBJS+=$(C_OBJ)

CFLAGS= \
	-DPOSIX=1 \
	-DSYS_CONSOLE_ENABLE \
	-DTCPIP_STACK_COMMAND_ENABLE \
	-fmessage-length=0  -fno-unwind-tables -fno-exceptions -fno-asynchronous-unwind-tables -fPIC \
	-MMD -MP

ifeq ($(DEBUG),)
CFLAGS+= \
	-Os -Wall
else
CFLAGS+= \
	-O0 -g3 -Wall
endif

INCLUDES:=\
	-I"$(DIR__SRC)" \
	-I"$(DIR__SDK)/Lib/include" \
	-I"$(DIR__STAGING)/include" \

# Each subdirectory must supply rules for building sources it contributes
$(C_OBJ) : $(DIR__OBJ)/%.o : $(DIR__SRC)/%.c | $(DIR__OBJ)
	@echo 'Building file: $<'
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

