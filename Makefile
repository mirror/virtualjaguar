# Makefile for Virtual Jaguar
#
# by James Hammons
# (C) 2011 Underground Software
#
# Note that we control the version information here--uncomment only one set of
# echo's from the "prepare" recipe. :-)
#

FIND = find

# Gah
OSTYPE := $(shell uname -a)

# Should catch both 'darwin' and 'darwin7.0'
ifeq "$(findstring Darwin,$(OSTYPE))" "Darwin"
QMAKE_EXTRA := -spec macx-g++
endif


all: prepare virtualjaguar
	@echo -e "\033[01;33m***\033[00;32m Success!\033[00m"

prepare:
	@echo -e "\033[01;33m***\033[00;32m Preparing to compile Virtual Jaguar...\033[00m"
#	@echo "#define VJ_RELEASE_VERSION \"v2.0.0\"" > src/version.h
#	@echo "#define VJ_RELEASE_SUBVERSION \"Final\"" >> src/version.h
	@echo "#define VJ_RELEASE_VERSION \"SVN `svn info | grep -i revision`\"" > src/version.h
	@echo "#define VJ_RELEASE_SUBVERSION \"2.0.0 Prerelease\"" >> src/version.h

virtualjaguar: sources libs makefile-qt
	@echo -e "\033[01;33m***\033[00;32m Making Virtual Jaguar...\033[00m"
	$(MAKE) -f makefile-qt

makefile-qt: virtualjaguar.pro
	@echo -e "\033[01;33m***\033[00;32m Creating Qt makefile...\033[00m"
	qmake $(QMAKE_EXTRA) virtualjaguar.pro -o makefile-qt

libs: obj/libmusashi.a
	@echo -e "\033[01;33m***\033[00;32m Libraries successfully made.\033[00m"

obj/libmusashi.a:
	@echo -e "\033[01;33m***\033[00;32m Making Musashi...\033[00m"
	$(MAKE) -f musashi.mak

sources: src/*.h src/*.cpp src/*.c

clean:
	@echo -ne "\033[01;33m***\033[00;32m Cleaning out the garbage...\033[00m"
	@-rm -rf ./obj
	@-rm makefile-qt
	@-rm virtualjaguar
	@-$(FIND) . -name "*~" -exec rm -f {} \;
	@echo "done!"

statistics:
	@echo -n "Lines in source files: "
	@-$(FIND) ./src -name "*.cpp" | xargs cat | wc -l
	@echo -n "Lines in header files: "
	@-$(FIND) ./src -name "*.h" | xargs cat | wc -l

dist:	clean
