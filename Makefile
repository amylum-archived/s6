PACKAGE = s6
ORG = amylum
BUILD_DIR = /tmp/$(PACKAGE)-build
RELEASE_DIR = /tmp/$(PACKAGE)-release
RELEASE_FILE = /tmp/$(PACKAGE).tar.gz

PACKAGE_VERSION = $$(awk -F= '/^version/ {print $$2}' upstream/package/info)
PATCH_VERSION = $$(cat version)
VERSION = $(PACKAGE_VERSION)-$(PATCH_VERSION)
CONF_FLAGS = --enable-allstatic --disable-shared --enable-static --enable-static-libc
PATH_FLAGS = --prefix=$(RELEASE_DIR) --exec-prefix=$(RELEASE_DIR)/usr --libdir=$(RELEASE_DIR)/usr/lib/s6 --includedir=$(RELEASE_DIR)/usr/include --libexecdir=$(RELEASE_DIR)/usr/lib --sbindir=$(RELEASE_DIR)/usr/bin

SKALIBS_VERSION = 2.3.8.3-36
SKALIBS_URL = https://github.com/amylum/skalibs/releases/download/$(SKALIBS_VERSION)/skalibs.tar.gz
SKALIBS_TAR = skalibs.tar.gz
SKALIBS_DIR = /tmp/skalibs
SKALIBS_PATH = --with-sysdeps=$(SKALIBS_DIR)/usr/lib/skalibs/sysdeps --with-lib=$(SKALIBS_DIR)/usr/lib/skalibs --with-include=$(SKALIBS_DIR)/usr/include --with-dynlib=$(SKALIBS_DIR)/usr/lib

EXECLINE_VERSION = 2.1.4.5-26
EXECLINE_URL = https://github.com/amylum/execline/releases/download/$(EXECLINE_VERSION)/execline.tar.gz
EXECLINE_TAR = execline.tar.gz
EXECLINE_DIR = /tmp/execline
EXECLINE_PATH = --with-lib=$(EXECLINE_DIR)/usr/lib/execline --with-include=$(EXECLINE_DIR)/usr/include --with-lib=$(EXECLINE_DIR)/usr/lib

.PHONY : default submodule manual container deps version build push local

default: submodule container

submodule:
	git submodule update --init

manual: submodule
	./meta/launch /bin/bash || true

container:
	./meta/launch

deps:
	rm -rf $(SKALIBS_DIR) $(EXECLINE_DIR) $(SKALIBS_TAR) $(EXECLINE_TAR)
	mkdir $(SKALIBS_DIR) $(EXECLINE_DIR)
	curl -sLo $(SKALIBS_TAR) $(SKALIBS_URL)
	tar -x -C $(SKALIBS_DIR) -f $(SKALIBS_TAR)
	curl -sLo $(EXECLINE_TAR) $(EXECLINE_URL)
	tar -x -C $(EXECLINE_DIR) -f $(EXECLINE_TAR)

build: submodule deps
	rm -rf $(BUILD_DIR)
	cp -R upstream $(BUILD_DIR)
	sed -i 's/0700/0755/' $(BUILD_DIR)/package/modes
	cd $(BUILD_DIR) && CC="musl-gcc" ./configure $(CONF_FLAGS) $(PATH_FLAGS) $(SKALIBS_PATH) $(EXECLINE_PATH)
	make -C $(BUILD_DIR)
	make -C $(BUILD_DIR) install
	mkdir -p $(RELEASE_DIR)/usr/share/licenses/$(PACKAGE)
	cp upstream/COPYING $(RELEASE_DIR)/usr/share/licenses/$(PACKAGE)/LICENSE
	cd $(RELEASE_DIR) && tar -czvf $(RELEASE_FILE) *

version:
	@echo $$(($(PATCH_VERSION) + 1)) > version

push: version
	git commit -am "$(VERSION)"
	ssh -oStrictHostKeyChecking=no git@github.com &>/dev/null || true
	git tag -f "$(VERSION)"
	git push --tags origin master
	@sleep 3
	targit -a .github -c -f $(ORG)/$(PACKAGE) $(VERSION) $(RELEASE_FILE)
	@sha512sum $(RELEASE_FILE) | cut -d' ' -f1

local: build push

