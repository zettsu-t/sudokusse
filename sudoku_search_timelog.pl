#!/bin/perl
# sudoku_solve_all.pl が出力したログから最短時間を抽出する
# 基準となる実行ファイルを指定すると、そこからどれだけ短縮できたか計算する
#
# 起動方法 : Cygwinのシェルから起動する
#   sudoku_search_timelog.pl sudoku_solve_all.plが出力したログファイル名
# 例 perl sudoku_search_timelog.pl SudokuTime_2013_09_27_21_34_56.log
#
# Copyright (C) 2013 Zettsu Tatsuya

use strict;

# 拡張子を付けないと起動できないことがある
my $exepostfix = ($^O eq "cygwin") ? ".exe" : "";

# sudoku_solve_all.pl と同様
my $EXE_SUDOKU_NOT_PACKED = "bin/sudokusse$exepostfix";
my $EXE_SUDOKU_PACKED = "bin/sudokusse_cells_packed$exepostfix";
my $EXE_SUDOKU_BASE = "ToBeReplaced$exepostfix";

my $SUDOKU_NOTHING = 0;
my $SUDOKU_PACKED = 1;
my $SUDOKU_NOT_PACKED = 2;
my $SUDOKU_BASE = 3;

my %EXE_MAP = ($SUDOKU_PACKED=>$EXE_SUDOKU_PACKED,
               $SUDOKU_NOT_PACKED=>$EXE_SUDOKU_NOT_PACKED,
               $SUDOKU_BASE=>$EXE_SUDOKU_BASE);

my $USEC_PER_SEC = 1000000.0;

(@ARGV > 0) || die "sudokutime.pl [log-filename]";
my $inFilename = @ARGV[0];
my %fastestTimeSecMap;

sub UpdateFastestTime {
    my ($type, $timeSec) = @_;
    if (!$fastestTimeSecMap{$type} || ($fastestTimeSecMap{$type} > $timeSec)) {
        $fastestTimeSecMap{$type} = $timeSec;
    }
    0;
}

sub ParseLogLine {
    my ($type, $line) = @_;
    my $timeSec;
    if ($line =~ /Total.* (\d+)usec.*/) {
        my $timeSec = 0.0 + ($1 / $USEC_PER_SEC);
        UpdateFastestTime($type, $timeSec);
    }
    0;
}

sub ParseBaseLogLine {
    # $EXE_SUDOKU_BASEの出力を解析する処理を、ここに追加する
    0;
}

sub ParseLogFile {
    my $type = $SUDOKU_NOTHING;
    my $line;

    open(IN, "< $inFilename") || die "cannot open $inFilename";
    while(<IN>) {
        $line = $_;
        chomp $line;
        $line =~ tr/\r\n//;

        if ($line =~ /^\s*$EXE_SUDOKU_PACKED\s*$/) {
            $type = $SUDOKU_PACKED;
        } elsif ($line =~ /^\s*$EXE_SUDOKU_NOT_PACKED\s*$/) {
            $type = $SUDOKU_NOT_PACKED;
        } elsif ($line =~ /^\s*$EXE_SUDOKU_BASE\s*$/) {
            $type = $SUDOKU_BASE;
        } else {
            if (($type == $SUDOKU_PACKED) || ($type == $SUDOKU_NOT_PACKED)) {
                ParseLogLine($type, $line);
            } elsif ($type == $SUDOKU_BASE) {
                ParseBaseLogLine($type, $line);
            }
        }
    }
    0;
}

sub PrintTime {
    my $baseTime = $fastestTimeSecMap{$SUDOKU_BASE};

    my $type;
    foreach $type (sort keys %EXE_MAP) {
        if ($type == $SUDOKU_NOTHING) {
            next;
        }

        my $fastestTime = $fastestTimeSecMap{$type};
        my $reducedTimeRatioStr = "";
        if ($baseTime) {
            my $reducedTimeRatio = (100.0 * ($fastestTime - $baseTime)) / $baseTime;
            $reducedTimeRatioStr = " (" . sprintf("%.1f", $reducedTimeRatio) . "%)";
        }

        print sprintf("%.3f", $fastestTime) . " [sec]";
        if ($type == $SUDOKU_BASE) {
            print " (base)";
        } else {
            print $reducedTimeRatioStr;
        }
        print " : " . $EXE_MAP{$type} . "\n";
    }
    0;
}

ParseLogFile();
PrintTime();
0;
