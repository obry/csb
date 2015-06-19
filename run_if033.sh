#!/bin/bash
# $Id: run_if033.sh 42 2015-04-25 11:46:48Z mfleck $:

prog=./uart_test
br=2000000
# for the Lattice board ttyUSB1, for the prototype motherboard ttyUSB0
# dev=/dev/ttyS13
dev=/dev/ttyUSB0
slave=7
dac=2000
outfile=test1.dat

$prog --dev $dev --br $br --slv $slave $* --thresh 0 0 0 100 --dac $dac
sleep 1
$prog --dev $dev --br $br --slv $slave $* --trigg -o $outfile
