# $Id: Makefile 68 2015-06-12 07:27:18Z angelov $:

CFLAGS = -g -Wall -DOS_LINUX

CC=g++

# fe, fe_bb or ctrl
func=fe

ifeq ($(func), fe)
prog_file=LCMXO2-2000ZE-1.jbt
rdback_file=fpga_fe.bit
device=/dev/ttyUSB0
slave=5
endif

ifeq ($(func), fe_bb)
prog_file=LCMXO2-1200ZE-1.jbt
rdback_file=fpga_fe_bb.bit
device=/dev/ttyUSB1
slave=7
endif

ifeq ($(func), ctrl)
prog_file=LCMXO2-7000HE-4.jbt
rdback_file=fpga_ctrl.bit
device=/dev/ttyUSB1
slave=12
endif

adc_file=adc.dat

#syntax: 
#target: dependencies
#[tab] system command


uart_test: uart_test.o crc8.o uart_crc.o uart.o one_if033.o
	g++ $(CFLAGS) -o $@ $^

uart_test.o: uart_test.cpp uart_crc.h uart.h one_if033.h
	g++ -c -Wall -pedantic -o $@ $<

crc8.o: crc8.cpp crc8.h
	g++ -Wall -pedantic $< -c -o $@

uart.o: uart.cpp uart.h
	g++ -Wall -pedantic $< -c -o $@

uart_crc.o: uart_crc.cpp uart_crc.h crc8.h uart.h
	g++ -Wall -pedantic $< -c -o $@

one_if033.o: one_if033.cpp one_if033.h uart_crc.h uart.h
	g++ -g -Wall -pedantic $< -c -o $@






prog:
	./uart_test --slv $(slave) --dev $(device) --thresh 1 0 0 0 --status --er_cfg --ucode timestamp --wr_cfg 0 $(prog_file) --status

prog8:
	./uart_test --slv 0 --dev $(device) --thresh 1 0 0 0 --status --er_cfg --ucode timestamp --wr_cfg 0 $(prog_file) --status
	./uart_test --slv 1 --dev $(device) --thresh 1 0 0 0 --status --er_cfg --ucode timestamp --wr_cfg 0 $(prog_file) --status
	./uart_test --slv 2 --dev $(device) --thresh 1 0 0 0 --status --er_cfg --ucode timestamp --wr_cfg 0 $(prog_file) --status
	./uart_test --slv 3 --dev $(device) --thresh 1 0 0 0 --status --er_cfg --ucode timestamp --wr_cfg 0 $(prog_file) --status
	./uart_test --slv 4 --dev $(device) --thresh 1 0 0 0 --status --er_cfg --ucode timestamp --wr_cfg 0 $(prog_file) --status
	./uart_test --slv 5 --dev $(device) --thresh 1 0 0 0 --status --er_cfg --ucode timestamp --wr_cfg 0 $(prog_file) --status
	./uart_test --slv 6 --dev $(device) --thresh 1 0 0 0 --status --er_cfg --ucode timestamp --wr_cfg 0 $(prog_file) --status
	./uart_test --slv 7 --dev $(device) --thresh 1 0 0 0 --status --er_cfg --ucode timestamp --wr_cfg 0 $(prog_file) --status

erase:
	./uart_test --slv $(slave) --dev $(device) --status --er_cfg --status

verify:
	./uart_test --slv $(slave) --dev $(device) --status --rd_cfg 0 $(rdback_file)
	diff -i -E -w -b -B --strip-trailing-cr -a -q $(prog_file) $(rdback_file)

verify8:
	./uart_test --slv 0 --dev $(device) --status --rd_cfg 0 $(rdback_file)
	diff -i -E -w -b -B --strip-trailing-cr -a -q $(prog_file) $(rdback_file)
	./uart_test --slv 1 --dev $(device) --status --rd_cfg 0 $(rdback_file)
	diff -i -E -w -b -B --strip-trailing-cr -a -q $(prog_file) $(rdback_file)
	./uart_test --slv 2 --dev $(device) --status --rd_cfg 0 $(rdback_file)
	diff -i -E -w -b -B --strip-trailing-cr -a -q $(prog_file) $(rdback_file)
	./uart_test --slv 3 --dev $(device) --status --rd_cfg 0 $(rdback_file)
	diff -i -E -w -b -B --strip-trailing-cr -a -q $(prog_file) $(rdback_file)
	./uart_test --slv 4 --dev $(device) --status --rd_cfg 0 $(rdback_file)
	diff -i -E -w -b -B --strip-trailing-cr -a -q $(prog_file) $(rdback_file)
	./uart_test --slv 5 --dev $(device) --status --rd_cfg 0 $(rdback_file)
	diff -i -E -w -b -B --strip-trailing-cr -a -q $(prog_file) $(rdback_file)
	./uart_test --slv 6 --dev $(device) --status --rd_cfg 0 $(rdback_file)
	diff -i -E -w -b -B --strip-trailing-cr -a -q $(prog_file) $(rdback_file)
	./uart_test --slv 7 --dev $(device) --status --rd_cfg 0 $(rdback_file)
	diff -i -E -w -b -B --strip-trailing-cr -a -q $(prog_file) $(rdback_file)

adc:
	./uart_test --slv $(slave) --dev $(device) --status --trigg --status -o $(adc_file)

adc8:
	./uart_test --slv 0 --dev $(device) --status --dacf 100 --trigg --status -o adc0.dat
	./uart_test --slv 1 --dev $(device) --status --dacf 200 --trigg --status -o adc1.dat
	./uart_test --slv 2 --dev $(device) --status --dacf 300 --trigg --status -o adc2.dat
	./uart_test --slv 3 --dev $(device) --status --dacf 400 --trigg --status -o adc3.dat
	./uart_test --slv 4 --dev $(device) --status --dacf 500 --trigg --status -o adc4.dat
	./uart_test --slv 5 --dev $(device) --status --dacf 600 --trigg --status -o adc5.dat
	./uart_test --slv 6 --dev $(device) --status --dacf 700 --trigg --status -o adc6.dat
	./uart_test --slv 7 --dev $(device) --status --dacf 800 --trigg --status -o adc7.dat

clean:
	rm *.o uart_test
