# SVN: $Revision: 476 $
VERSION=0.9

FONT_TEXT=/usr/share/fonts/truetype/msttcorefonts/Arial.ttf
FONT_TITLE=/usr/share/fonts/truetype/msttcorefonts/Arial.ttf

DEBUG= -ggdb3 #-D_DEBUG #-fprofile-arcs -ftest-coverage # -pg -g
CXXFLAGS+=-O3 -Wall -DVERSION=\"${VERSION}\" $(DEBUG) -DFONT_TEXT=\"$(FONT_TEXT)\" -DFONT_TITLE=\"$(FONT_TITLE)\" -DBUILD_BY=\"${shell echo `whoami`@`hostname`}\"
LDFLAGS+=$(DEBUG) -lsqlite3
LDFLAGSco=-pthread -lncurses -lgps
LDFLAGSmd=-lgd
LDFLAGSev=-lhpdf -lcurl -ltinyxml2 -lfftw3
LDFLAGSwg=-pthread -lgps
LDFLAGSem=

OBJS=error-nc.o byte_array.o utils.o serial_io.o serial_io_tcpip.o IO_Device.o IO_ELM327.o IO_Test.o OBD2.o sensors/sensor.o sensors/io_sensor.o terminal.o terminal_console.o terminal_ncurses.o gps.o gpsd.o main.o sensors/sensor_fuel_system_status.o sensors/sensor_distance_with_mil_on.o sensors/sensor_warmups_after_code_cleared.o sensors/sensor_obd2_standards.o sensors/sensor_fuel_type.o obd2-emulation.o
OBJSmd=error.o utils.o utils-sp.o graph.o cluster_data.o kml.o xml.o main-dump.o
OBJSev=error.o utils.o utils-sp.o logo.o graph_pdf.o fft.o main-evaluate.o
OBJSwg=error.o utils.o gps.o gpsd.o terminal.o terminal_console.o serial_io.o w4gf.o
OBJSem=error.o byte_array.o utils.o terminal.o terminal_console.o obd2-emulation.o main-elm327-emulator.o

all: O2OO-collector O2OO-dumper O2OO-evaluate O2OO-w4gf O2OO-elm327-emulator

O2OO-collector: $(OBJS)
	$(CXX) -Wall -W $(OBJS) $(LDFLAGS) $(LDFLAGSco) -o O2OO-collector

O2OO-dumper: $(OBJSmd)
	$(CXX) -Wall -W $(OBJSmd) $(LDFLAGS) $(LDFLAGSmd) -o O2OO-dumper

O2OO-evaluate: $(OBJSev)
	$(CXX) -Wall -W $(OBJSev) $(LDFLAGS) $(LDFLAGSev) -o O2OO-evaluate

O2OO-w4gf: $(OBJSwg)
	$(CXX) -Wall -W $(OBJSwg) $(LDFLAGS) $(LDFLAGSwg) -o O2OO-w4gf

O2OO-elm327-emulator: $(OBJSem)
	$(CXX) -Wall -W $(OBJSem) $(LDFLAGS) $(LDFLAGSem) -o O2OO-elm327-emulator

install: O2OO-collector O2OO-dumper O2OO-evaluate O2OO-w4gf O2OO-elm327-emulator
	cp O2OO-collector /usr/local/bin
	cp O2OO-dumper /usr/local/bin
	cp O2OO-evaluate /usr/local/bin
	cp O2OO-w4gf /usr/local/bin
	cp O2OO-elm327-emulator /usr/local/bin

clean:
	rm -f $(OBJS) O2OO-collector $(OBJSmd) O2OO-dumper $(OBJSev) O2OO-evaluate $(OBJSwg) O2OO-w4gf $(OBJSem) O2OO-elm327-emulator core *.da *.gcov *.bb*

package: clean
	mkdir O2OO-$(VERSION) O2OO-$(VERSION)/sensors
	cp *.cpp *.h Makefile readme.txt O2OO-$(VERSION)
	cp sensors/*cpp sensors/*h O2OO-$(VERSION)/sensors/
	tar czf O2OO-$(VERSION).tgz O2OO-$(VERSION)
	rm -rf O2OO-$(VERSION)

check:
	cppcheck -v --enable=all --std=c++11 --inconclusive -I. . 2> err.txt
