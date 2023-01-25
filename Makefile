APP_ID:=lol.es3gears
VERSION:=0.0.1
MAIN:=main
ICON:=icon_80x80.png
APP_DIR:=app-build
IPK:=$(APP_ID)_$(VERSION)_arm.ipk

export CC:=arm-starfishmllib32-linux-gnueabi-gcc --sysroot=/opt/starfish-sdk-x86_64/5.0.0-20191125/sysroots/aarch64-starfish-linux \
    -mcpu=cortex-a55 -mfloat-abi=softfp -mthumb -rdynamic -funwind-tables
export CFLAGS:=-pipe -std=gnu17 -Wall -Wextra -O2 -ggdb -feliminate-unused-debug-types -DDEFAULT_APP_ID='"$(APP_ID)"'
export LDFLAGS:=-Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed

LIBS:=-lm -lwayland-client -lwayland-egl -lwayland-webos-client -lmali

.PHONY: all
all: $(IPK)

$(MAIN): main.c es3gears.c eglut/libeglut.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o '$@' $^ -Ieglut $(LIBS)

appinfo.json: appinfo.json.in Makefile
	sed -e 's/@APP_ID@/$(APP_ID)/g' \
	    -e 's/@MAIN@/$(MAIN)/g' \
	    -e 's/@VERSION@/$(VERSION)/g' < '$<' > '$@'

eglut/libeglut.a: eglut

.PHONY: eglut
eglut:
	 $(MAKE) -C eglut -- libeglut.a

$(IPK): main appinfo.json $(ICON)
	mkdir -p -- '$(APP_DIR)'
	cp -t '$(APP_DIR)' -- $^
	ares-package '$(APP_DIR)'

.PHONY: install
install:
	ares-install '$(IPK)'

.PHONY: launch
launch:
	ares-launch '$(APP_ID)'

.PHONY: clean
clean:
	rm -f -- '$(MAIN)' '$(IPK)' appinfo.json
	rm -rf -- '$(APP_DIR)'
	$(MAKE) -C eglut -- clean
