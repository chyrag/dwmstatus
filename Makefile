TARGET = dwmstatus
LDFLAGS += -lX11
CFLAGS += -DDEBUG -ggdb -Wall -Werror
#CFLAGS += -O2 -s -Wall -Werror

all: $(TARGET)

install: $(TARGET)
	install -s $^ $(HOME)/bin/

clean:
	rm -f $(TARGET)
