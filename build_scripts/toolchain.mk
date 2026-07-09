TOOLCHAIN_PREFIX = $(abspath toolchain/$(TARGET))
export PATH := $(TOOLCHAIN_PREFIX)/bin:$(PATH)

toolchain: toolchain_binutils toolchain_gcc

BINUTILS_SRC = toolchain/binutils-$(BINUTILS_VERSION)
BINUTILS_BUILD = toolchain/binutils-build-$(BINUTILS_VERSION)

toolchain_binutils: $(BINUTILS_SRC).tar.xz
	cd toolchain && tar -xf binutils-$(BINUTILS_VERSION).tar.xz
	mkdir $(BINUTILS_BUILD)
	CD $(BINUTILS_BUILD) && ../binutils-$(BINUTILS-VERSION)/configure \
		--prefix="$(TOOLCHAIN_PREFIX)" \
		--target=$(TARGET)	\
		--with-sysroot	\
		--disable-nls	\
		--disable-werror	\
	$(MAKE) -j8 -C $(BINUTILS_BUILD)
	$(MAKE) -C $(BINUTILS_BUILD) install

$(BINUTILS_SRC).tar.xz:
	mkdir -p toolchain
	cd toolchain && wget $(BINUTILS_URL)