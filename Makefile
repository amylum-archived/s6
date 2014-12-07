PACKAGE = s6
BUILD_DIR = /tmp/$(PACKAGE)-build
RELEASE_DIR = /tmp/$(PACKAGE)-release
RELEASE_FILE = $(PACKAGE).tar.gz

SKALIBS_VERSION = 1.6.0.0
SKALIBS_URL = https://github.com/akerl/skalibs/releases/download/$(SKALIBS_VERSION)/skalibs.tar.gz
SKALIBS_TAR = skalibs.tar.gz
SKALIBS_DIR = /tmp/skalibs

EXECLINE_VERSION = 1.3.1.1
EXECLINE_URL = https://github.com/akerl/execline/releases/download/$(EXECLINE_VERSION)/execline.tar.gz
EXECLINE_TAR = execline.tar.gz
EXECLINE_DIR = /tmp/execline

.PHONY : default manual container build push local

default: container

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
	patch -d $(BUILD_DIR) -p1 < patch
	make -C $(BUILD_DIR) install
	tar -czv -C $(RELEASE_DIR) -f $(RELEASE_FILE) .

push:
	git commit -am "$$(cat upstream/package/version)" || true
	ssh -oStrictHostKeyChecking=no git@github.com &>/dev/null || true
	git tag -f "$$(cat upstream/package/version)"
	git push origin :"$$(cat upstream/package/version)" || true
	git push --tags origin master
	targit -a .github -c -f akerl/s6 $$(cat upstream/package/version) $(RELEASE_FILE)

local: build push

