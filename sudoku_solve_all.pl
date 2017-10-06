#!/bin/perl
# This script solves all sudoku puzzles in data/* files,
# checks whether they are correctly solved, and measures
# how long time does it takes.
#
# Usage: Launch this script from Cygwin terminal without arguments.
#   $ perl sudoku_solve_all.pl
#
# Copyright (C) 2012-2017 Zettsu Tatsuya

use strict;

# It may be required to launch executables with ".exe" suffix on Windows.
my $exepostfix = ($^O eq "cygwin") ? ".exe" : "";

my $EXE_FILENANE = "bin/sudokusse$exepostfix";
my $OUT_OF_STEP = 0;
my $IN_STEP = 1;

sub trimCrLF {
    my ($line) = @_;
    chomp($line);
    $line =~ tr/\r\n//;
    $line;
}

sub checkAnswer {
    my ($numstr) = @_;

    my @numarray;
    my $i;
    for($i=0; $i<81; $i+=1) {
        @numarray[$i] = "";
    }

    # Collect cells
    for($i=0; $i<81; $i+=1) {
        my $rowbase = int($i / 9) * 9;
        my $columnbase = $i % 9;
        my $boxlumnbase = int($i / 27) * 27 + int($columnbase / 3) * 3;
        my $num = substr($numstr,$i,1);
        $num = (!$num) ? "." : $num;

        my $j;
        for($j=0; $j<9; $j+=1) {
            @numarray[$rowbase + $j] .= $num;
            @numarray[$columnbase + $j*9] .= $num;
        }
        for($j=0; $j<3; $j+=1) {
            @numarray[$boxlumnbase + $j] .= $num;
            @numarray[$boxlumnbase + 9 + $j] .= $num;
            @numarray[$boxlumnbase + 18 + $j] .= $num;
        }
    }

    # Expect each row, column and box have three 1 to 9.
    for($i=0; $i<81; $i+=1) {
        my $actual = join("",sort(split(//,@numarray[$i])));
        ($actual eq "111222333444555666777888999") || die "not filled entry $actual : $i";
    }

    0;
}

sub checkResult {
    my ($filename) = @_;
    my @outstr = `$EXE_FILENANE -1 < $filename`;
    my $state = $OUT_OF_STEP;
    my $lastStepStr;
    my $lastNumStr;
    my $currentStepStr;
    my $currentNumStr;
    my $foundEmpty;
    my $popcnt;
    my $answercnt;
    my $solver;

    my $outstrline;
    foreach $outstrline (@outstr) {
        my $line = trimCrLF($outstrline);
        # Beginning of solutions
        if ($line =~ /^\d*:\d*:\d*:\d*:\d*:\d*:\d*:\d*:\d*:\s*$/) {
            if ($state == $OUT_OF_STEP) {
                $state = $IN_STEP;
                $currentStepStr = "";
                $currentNumStr = "";
                $foundEmpty = 0;
                $popcnt = 0;
            }

            $currentStepStr .= $outstrline;
            if ($line !~ /^\d+:\d+:\d+:\d+:\d+:\d+:\d+:\d+:\d+:\s*$/) {
                $foundEmpty = 1;
            }

            # Counts cells
            $line =~ s/\D//g;
            $currentNumStr .= $line;
            my $count = length($line);
            $popcnt += $count;
            next;
        }

        if ($state == $IN_STEP) {
            $lastStepStr = $currentStepStr;
            $lastNumStr = $currentNumStr;
            $state = $OUT_OF_STEP;
        }

        if ($line =~ /^\[(.*)\]\s*$/) {
            $solver = $1;
        }

        # Checks a footer of output lines
        if ($line =~ /^Total/) {
            $answercnt += 1;
            print "-------- $filename result ($answercnt, solver = $solver) --------\n";
            print $lastStepStr;
            print "found empty = $foundEmpty, filled entries = $popcnt\n";
            (($foundEmpty) || ($popcnt != 81)) && die "not solved";
            checkAnswer($lastNumStr);
        }
    }
    0;
}

sub solve {
    my ($count) = @_;

    # Collects input files
    my @outstr = `find data -name "*.txt"`;
    my @files;

    my $outstrline;
    # Check whether the solver solves correctly
    foreach $outstrline (@outstr) {
        my $filename = trimCrLF($outstrline);
        push(@files, $filename);
        checkResult($filename);
    }

    # Measures elapsed time
    my $filename;
    foreach $filename (@files) {
        print "\n-------- $filename time begin --------\n";
        system("$EXE_FILENANE $count < $filename");
        print "-------- $filename time end ----------\n";
    }

    0;
}

my $count = 10000;

if (@ARGV > 0) {
    my $arg = @ARGV[0];
    if ($arg > 0) {
        $count = $arg;
    }
}

solve($count);
