#!/bin/perl
# 数独の解を数える(時間計測)
# 繰り返し実行して、標準出力と、時刻を基にしたファイルにログを出力する
#
# 起動方法 : Cygwinのシェルから起動する
#   sudoku_count.pl 数独マップのファイル名
# 例 perl sudoku_count.pl data/sample.txt
#   SudokuTime_2013_09_27_21_34_56.log などのログファイルが作成される
#
# Copyright (C) 2013 Zettsu Tatsuya

use strict;

# 拡張子を付けないと起動できないことがある
my $exepostfix = ($^O eq "cygwin") ? ".exe" : "";

# 数独の解を数える実行ファイル名
# 最初に埋まっているマスは左上に固まっていると仮定しない
my $EXE_SUDOKU_NOT_PACKED = "bin/sudokusse$exepostfix";
# 同仮定する
my $EXE_SUDOKU_PACKED = "bin/sudokusse_cells_packed$exepostfix";

# 実行時間を比較する対象のプログラム
# 名前は適宜変更する
# my $EXE_SUDOKU_BASE = "./ToBeReplaced$exepostfix";

# 引数はサニタイズしていない
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

# 途中で終了するにはCtrl-cを連打する
while(1) {
    my $options = "0 < " . "'" . "$inFilename" . "'";
    launch($EXE_SUDOKU_PACKED, $options);
    launch($EXE_SUDOKU_NOT_PACKED, $options);
    $options = "'" . "$inFilename" . "'";
    # $EXE_SUDOKU_BASEにオプションを付けて起動するときは、ここに追加する
}

0;
