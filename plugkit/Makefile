ELECTRON_VERSION = $(shell jq '.devDependencies."electron-deplug"' ../../package.json -r)
GYP=../../node_modules/.bin/node-gyp

ifeq ($(OS),Windows_NT)
CONFIG = build/binding.sln
else
CONFIG = build/Makefile
ENV = JOBS=4 HOME=~/.electron-gyp
endif

all: $(CONFIG)
	$(ENV) $(GYP) build --target=$(ELECTRON_VERSION) \
		--arch=x64 --dist-url=https://atom.io/download/electron

$(CONFIG):
	$(ENV) $(GYP) configure --target=$(ELECTRON_VERSION) \
		--arch=x64 --dist-url=https://atom.io/download/electron

clean:
	@$(GYP) clean

fmt:
	@clang-format -i src/*.cpp src/*.hpp \
		include/plugkit/*.hpp src/private/*.hpp \
		src/wrapper/*.hpp src/wrapper/*.cpp \
		**/*.hpp **/*.cpp

.PHONY: all clean fmt