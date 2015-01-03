PACKAGE = s6
ORG = amylum
BUILD_DIR = /tmp/$(PACKAGE)-build
RELEASE_DIR = /tmp/$(PACKAGE)-release
RELEASE_FILE = /tmp/$(PACKAGE).tar.gz

PACKAGE_VERSION = $$(awk -F= '/^version/ {print $$2}' upstream/package/info)
PATCH_VERSION = $$(cat version)
VERSION = $(PACKAGE_VERSION)-$(PATCH_VERSION)

SKALIBS_VERSION = 2.0.0.0-11
SKALIBS_URL = https://github.com/amylum/skalibs/releases/download/$(SKALIBS_VERSION)/skalibs.tar.gz
SKALIBS_TAR = skalibs.tar.gz
SKALIBS_DIR = /tmp/skalibs
SKALIBS_PATH = --with-sysdeps=$(SKALIBS_DIR)/usr/lib/skalibs/sysdeps --with-lib=$(SKALIBS_DIR)/usr/lib/skalibs --with-include=$(SKALIBS_DIR)/usr/include --with-dynlib=$(SKALIBS_DIR)/usr/lib

EXECLINE_VERSION = 2.0.0.0-6
EXECLINE_URL = https://github.com/amylum/execline/releases/download/$(EXECLINE_VERSION)/execline.tar.gz
EXECLINE_TAR = execline.tar.gz
EXECLINE_DIR = /tmp/execline
EXECLINE_PATH = --with-lib=$(EXECLINE_DIR)/usr/lib/execline --with-include=$(EXECLINE_DIR)/usr/include --with-dynlib=$(EXECLINE_DIR)/usr/lib

.PHONY : default manual container deps version build push local

default: upstream/Makefile container

upstream/Makefile:
	git submodule update --init

manual:
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

build: deps
	rm -rf $(BUILD_DIR)
	cp -R upstream $(BUILD_DIR)
	cd $(BUILD_DIR) && CC="musl-gcc" ./configure --prefix=$(RELEASE_DIR) $(CONF_FLAGS) $(SKALIBS_PATH) $(EXECLINE_PATH)
	make -C $(BUILD_DIR)
	make -C $(BUILD_DIR) install
	cd $(RELEASE_DIR) && tar -czvf $(RELEASE_FILE) *

version:
	@echo $$(($(PATCH_VERSION) + 1)) > version

push: version
	git commit -am "$(VERSION)"
	ssh -oStrictHostKeyChecking=no git@github.com &>/dev/null || true
	git tag -f "$(VERSION)"
	git push --tags origin master
	targit -a .github -c -f $(ORG)/$(PACKAGE) $(VERSION) $(RELEASE_FILE)

local: build push

