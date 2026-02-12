SRCTREE ?= ../../../../

.PHONY: clean all install

all:
	cmake $(CURDIR) -G Ninja -Bbuild -DCMAKE_TOOLCHAIN_FILE=$(SRCTREE)/cmake/toolchain.cmake
	ninja -C $(CURDIR)/build/$(CVILIB_DIR)

install:
	ninja -C $(CURDIR)/build/$(CVILIB_DIR) install

clean:
	rm $(CURDIR)/build -rf
	rm $(CURDIR)/install -rf
