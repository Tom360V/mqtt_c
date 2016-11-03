###############################################################################
# Enter AppName
###############################################################################
APPNAME=mqttClient_x86

###############################################################################
# ADD SOURCES
###############################################################################
C_SRC= \
main.c \
clientSock.c \
x86.c \
src/PubSubClient.c \

CPP_SRC= \


###############################################################################
# ADD INCLUDE
###############################################################################
INCL:= \
-I . \
-I inc \
-I src \

#-I /c/Program\ Files\ \(x86\)/Arduino/hardware/arduino/avr/cores/arduino \
#-I /c/Program\ Files\ \(x86\)/Arduino/hardware/tools/avr/avr/include \

LDFLAGS:= \

OUTPUTDIR:=output
###############################################################################
###############################################################################
###############################################################################
APP:=$(OUTPUTDIR)/$(APPNAME).exe
MAPFILE:=$(OUTPUTDIR)/$(APPNAME).map
HEADER="**********"
BASE_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BASE_DIR:=$(subst test/,,$(BASE_DIR))
OUTPUTDIR:=$(abspath output)
#INCL:=$(addprefix -I ,$(INCL))
OBJS:=$(C_SRC:.c=.c.o)
OBJS+=$(CPP_SRC:.cpp=.cpp.o)
OBJS:=$(addprefix $(OUTPUTDIR)/,$(OBJS))
CC:=g++

all: pre $(APP)
	@echo $(HEADER)
	@echo done

clean:
	@echo $(HEADER)
	@echo Clean
	rm -rf $(OUTPUTDIR)

rebuild: clean all

.PHONY: clean pre

pre:
	@echo $(HEADER)
	@echo "TARGET:   " $(APP)
	@echo "BASE_DIR: " $(BASE_DIR)
	@echo "OUTPUTDIR:" $(OUTPUTDIR)
	@echo "FLAGS:    " $(LDFLAGS)
	@echo "OBJS:"
	@echo -e  $(foreach dir,$(OBJS), "\r           $(dir)\r\n")
	@echo ""

$(OUTPUTDIR)%.c.o : $(BASE_DIR)%.c
	@echo $(HEADER)
	@echo "Build C: $< as $@"
	@mkdir -p $(dir $@)
	$(CC) -g3 -O0 -Wall -Wl,-Map=$@.map -Wl,--cref ${INCL} -c $^ -o $@
	size $@
	@echo ' '

$(OUTPUTDIR)%.cpp.o : $(BASE_DIR)%.cpp
	@echo $(HEADER)
	@echo "Build C: $< as $@"
	@mkdir -p $(dir $@)
	$(CC) -g3 -O0 -Wall -Wl,-Map=$@.map -Wl,--cref ${INCL} -c $^ -o $@
	size $@
	@echo ' '

$(APP): $(OBJS)
	@echo $(HEADER)
	@echo "Build APP: $@"
	@echo "Use: $^"
	$(CC) -g3 -O0 -Wall $(LDFLAGS) $^ -o $@ -lws2_32 -Wl,-Map,$(MAPFILE)
	size $(APP)
