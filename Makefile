DEPLUG_CORE = node_modules/deplug-core

DEPLUG_CORE_RES = $(wildcard deplug-core/*.htm) $(wildcard deplug-core/*.less)
DEPLUG_CORE_RES_OUT = $(addprefix node_modules/,$(DEPLUG_CORE_RES))

DEPLUG_CORE_JS = $(wildcard deplug-core/*.main.js)
DEPLUG_CORE_JS_OUT = $(addprefix node_modules/,$(DEPLUG_CORE_JS))

ROOLUP_EXTERN = electron,deplug,$(shell jq '.dependencies | keys | join(",")' package.json -r)
ROLLUP = node_modules/.bin/rollup

ELECTRON = node_modules/.bin/electron-deplug
ELECTRON_VERSION = $(shell jq '.devDependencies."electron-deplug"' package.json -r)
ELECTRON_MIRROR = https://s3-ap-northeast-1.amazonaws.com/deplug-build-junk/electron/v
ELECTRON_UNPACK = node_modules/deplug-helper

PACKAGER = node_modules/.bin/electron-packager

all: build
	$(ELECTRON) .

build: $(DEPLUG_CORE_RES_OUT) $(DEPLUG_CORE_JS_OUT)

pack: build
	$(PACKAGER) ./ --download.mirror=$(ELECTRON_MIRROR) \
	 						--asar.unpackDir=$(ELECTRON_UNPACK) \
							--electron-version=$(ELECTRON_VERSION) \
							--out=./out --overwrite

$(DEPLUG_CORE_RES_OUT): $(DEPLUG_CORE_RES) $(DEPLUG_CORE)
	cp $(subst node_modules/,,$@) $@

$(DEPLUG_CORE_JS_OUT): $(DEPLUG_CORE_JS) $(DEPLUG_CORE)
	$(ROLLUP) $(subst node_modules/,,$@) -e $(ROOLUP_EXTERN) --format cjs --output $@

$(DEPLUG_CORE):
	@mkdir $(DEPLUG_CORE)

.PHONY: all run build pack clean fmt
