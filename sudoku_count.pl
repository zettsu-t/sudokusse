#!/bin/perl
# This script counts solutions of a sudoku puzzle and measures time
# how long time does it takes until stopped. To stop this script,
# repeat hitting Ctrl-c.
#
# Usage: Launch this script from Cygwin terminal.
#   $ perl sudoku_count.pl filename
# The argument filename indicates a file in which a sudoku puzzle is.
#   $ perl sudoku_count.pl data/sample.txt
# This script creates a log file. Its name is based on the current time
# such as SudokuTime_2013_09_27_21_34_56.log .
#
# Copyright (C) 2013-2017 Zettsu Tatsuya

use strict;

# It may be required to launch executables with ".exe" suffix on Windows.
my $exepostfix = ($^O eq "cygwin") ? ".exe" : "";

# Names of executable files
# The file below does not assume that preset cells are not packed at top-left.
my $EXE_SUDOKU_NOT_PACKED = "bin/sudokusse$exepostfix";
# The file below assumes that preset cells are packed at top-left.
my $EXE_SUDOKU_PACKED = "bin/sudokusse_cells_packed$exepostfix";

# Notice arguments are not sanitized.
(@ARGV > 0) || die "sudokutime.pl [sudoku-filename]";
my $inFilename = @ARGV[0];

(-f $inFilename) || die "$inFilename does not exist";

my $dateString = `date --rfc-3339=seconds`;
$dateString =~ /^(.*)\+.*$/;
$dateString = $1;
$dateString =~ s/\D/_/g;
my $logFilename = "SudokuTime_" . $dateString . ".log";

sub launch {
    my ($exeFilename, $options) = @_;
    print "$exeFilename" . "\n";
    system("echo '' >> $logFilename");
    system("echo $exeFilename >> $logFilename");
    system("$exeFilename $options | tee -a $logFilename");
}

while(1) {
    my $options = "0 < " . "'" . "$inFilename" . "'";
    launch($EXE_SUDOKU_PACKED, $options);
    launch($EXE_SUDOKU_NOT_PACKED, $options);
    $options = "'" . "$inFilename" . "'";
}

0;
