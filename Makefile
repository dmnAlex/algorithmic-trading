PROJECTNAME=AlgorithmicTrading

CC=g++
CXXFLAGS=-c -fPIC -Wall -Werror -Wextra -Wpedantic -std=c++17 `pkg-config --cflags Qt5Gui`
LDFLAGS=`pkg-config --libs Qt5Gui` -lgtest -lm -pthread

SRCFILES=src/view_model/*.cc src/model/*.cc
HDRFILES=$(SRCFILES:.cc=.h)

INSTALLDIR=build
EXECUTABLE=$(PROJECTNAME)

REPORTDIR=report
LEAKS_REPORT_FILE=leaks_report.txt
LEAK_TEST=valgrind --leak-check=full --verbose --log-file=$(REPORTDIR)/$(LEAKS_REPORT_FILE)


.PHONY: all install uninstall clean dvi dist style cppcheck

all: install

%.o: %.cc
	$(CC) $(CXXFLAGS) $^ -o $@

install: uninstall
	@cmake -S . -B $(INSTALLDIR) && cmake --build $(INSTALLDIR) 

uninstall:
	@rm -rf $(INSTALLDIR)

clean:
	@rm -f $(EXECUTABLE)
	@rm -rf $(REPORTDIR) $(INSTALLDIR)
	@find . -type f -name "*.o" -exec rm -f {} \;

dist: install
	@cd $(INSTALLDIR) && tar -czf $(PROJECTNAME).tgz $(PROJECTNAME) && rm -f $(PROJECTNAME)

leaks:
	$(LEAK_TEST) ./$(EXECUTABLE)

style:
	clang-format -n -style=google src/main.cc $(SRCFILES) $(HDRFILES)

cppcheck:
	@cppcheck --language=c++ --std=c++17 --enable=all --suppress=unusedFunction  --suppress=noExplicitConstructor \
	--suppress=missingInclude --suppress=unknownMacro src/main.cc $(SRCFILES) $(HDRFILES)
