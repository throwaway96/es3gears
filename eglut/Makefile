LIB:=libeglut.a
OBJS:=eglut.o eglut_wayland.o

.PHONY: all
all: $(LIB)

$(LIB): $(OBJS)
	$(AR) rcs '$@' $^

.PHONY: clean
clean:
	rm -f -- '$(LIB)' $(OBJS)
