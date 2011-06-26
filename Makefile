# Makefile for Virtual Jaguar
#
# by James Hammons
# (C) 2011 Underground Software

FIND = find

all: prepare virtualjaguar
	@echo -e "\033[01;33m***\033[00;32m Success!\033[00m"

prepare:
	@echo -e "\033[01;33m***\033[00;32m Preparing to compile Virtual Jaguar...\033[00m"

virtualjaguar: sources libs makefile-qt
	@echo -e "\033[01;33m***\033[00;32m Making Virtual Jaguar...\033[00m"
	$(MAKE) -f makefile-qt

makefile-qt: virtualjaguar.pro
	@echo -e "\033[01;33m***\033[00;32m Creating Qt makefile...\033[00m"
	qmake virtualjaguar.pro -o makefile-qt

libs: obj/libmusashi.a
	@echo -e "\033[01;33m***\033[00;32m Make libs here (if any)...\033[00m"

obj/libmusashi.a:
	@echo -e "\033[01;33m***\033[00;32m Making Musashi...\033[00m"
	$(MAKE) -f musashi.mak

sources: src/*.h src/*.cpp src/*.c

clean:
	@echo -ne "\033[01;33m***\033[00;32m Cleaning out the garbage...\033[00m"
#	@-$(MAKE) -f makefile-qt clean
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
