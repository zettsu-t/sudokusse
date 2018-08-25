#!/usr/bin/ruby
# -*- coding: utf-8 -*-
#
# Testing executables
# Copyright (C) 2017 Zettsu Tatsuya
#
# usage : ruby sudoku_check.rb [puzzleFilename [execFilename]]

require 'rbconfig'

# Detect MinGW
RUN_ON_MINGW = RbConfig::CONFIG['host_os'].include?("mswin") || RbConfig::CONFIG['host_os'].include?("mingw")

# Arguments, their descriptions and stdout messages
# Solve by C++ or SSE/AVX
CODE_ARGUMENT_SET = ["avx", "c++"]
CODE_DESCRIPTION_SET = ["SIMD", "C++"]

# Solve parallel or not
PARALLEL_ARGUMENT_SET = [" ", "-N"]
PARALLEL_ARGUMENT_RUST_SET = ["-1", ""]
PARALLEL_DESCRIPTION_SET = ["Single thread", "Multi-threads"]

# Print or check sudoku solutions, or just solving
MODE_PRINT_SOLUTIONS = "print"
MODE_ARGUMENT_SET = [MODE_PRINT_SOLUTIONS, "on", "off"]
MODE_ARGUMENT_RUST_SET = ["", "-s -v", "-s"]
MODE_DESCRIPTION_SET = ["print solutions", "check solutions", "solve only"]
MODE_RESULT_SET = ["passed", "passed", "solved"]

# Minimum number of lines in a log
NUMBER_OF_LINES=2

# Download and save this file to test
# http://staffhome.ecm.uwa.edu.au/~00013890/sudoku17
# (Web browsers may save it as sudoku17.txt and you have to
#  rename it to sudoku17 after download it)

PRIMARY_DEFAULT_PUZZLE_FILENAME = "data/sudoku17.txt"
ALT_DEFAULT_PUZZLE_FILENAME = "data/sudoku17"

# SudokuSSE executable name
DEFAULT_EXEC_FILENAME = RUN_ON_MINGW ? "bin\\sudokusse.exe" : "bin/sudokusse"

# Sudoku Rust executable name
RUST_EXEC_FILENAME = "sudoku_rust/target/release/sudoku_rust"

# Log file name for each execution
TEMPORARY_LOGFILE_NAME="./_sudoku_temp_log.txt"

# Report an error
class SudokuSolverError < StandardError
end

class Launcher
  def initialize(execFilename, puzzleFilename, logFilename, resultKeyword, numberOfLines, optionSet)
    @execFilename = execFilename
    @puzzleFilename = puzzleFilename
    @logFilename = logFilename
    @resultKeyword = resultKeyword
    @numberOfLines = numberOfLines

    argSet = puzzleFilename + " " +  optionSet.join(" ")
    @command = "#{@execFilename} #{argSet} > #{@logFilename}"
  end

  def exec
    startTime = Time.now
    status = system(@command)
    stopTime = Time.now
    abortExec unless status

    if @resultKeyword
      lastLine = `tail -2 #{@logFilename}`
      abortExec unless lastLine.include?(@resultKeyword)
    end

    num = (`wc -l #{@logFilename}`.split)[0].to_i
    abortExec unless num == @numberOfLines

    sprintf("%8.5f", stopTime - startTime)
  end

  def abortExec
    raise SudokuSolverError, "Failed #{@command}"
  end
end

class ParameterSet
  def initialize(puzzleFilename, execFilename, logFilename)
    raise SudokuSolverError unless File.exist?(puzzleFilename)
    raise SudokuSolverError unless File.exist?(execFilename)

    @puzzleFilename = puzzleFilename
    @execFilename = execFilename
    @logFilename = logFilename
    @numberOfPuzzles = `wc -l #{@puzzleFilename}`.split[0].to_i

    @codeStrSet = CODE_ARGUMENT_SET
    @parallelStrSet = PARALLEL_ARGUMENT_SET
    @parallelStrRustSet = PARALLEL_ARGUMENT_RUST_SET
    @modeStrSet = MODE_ARGUMENT_SET
    @modeStrRustSet = MODE_ARGUMENT_RUST_SET

    @codeStrMap = makeMap(@codeStrSet, CODE_DESCRIPTION_SET)
    @parallelStrMap = makeMap(@parallelStrSet, PARALLEL_DESCRIPTION_SET)
    @parallelStrRustMap = makeMap(@parallelStrRustSet, PARALLEL_DESCRIPTION_SET)
    @modeStrMap = makeMap(@modeStrSet, MODE_DESCRIPTION_SET)
    @modeStrRustMap = makeMap(@modeStrRustSet, MODE_DESCRIPTION_SET)
  end

  def execAll
    puts "Solving #{@puzzleFilename} with #{@execFilename} (time in seconds)"
    resultMap = Hash[MODE_ARGUMENT_SET.zip(MODE_RESULT_SET)]

    @codeStrSet.each do |code|
      @parallelStrSet.each do |parallel|
        @modeStrSet.each do |mode|
          numberOfLogLines = (mode == MODE_PRINT_SOLUTIONS) ? @numberOfPuzzles : 0
          resultKeyword = resultMap.fetch(mode, nil)
          optionSet = [code, parallel, mode]
          desc = [@codeStrMap[code], @parallelStrMap[parallel], @modeStrMap[mode]].join(" / ") + " :"
          print desc
          puts exec(NUMBER_OF_LINES + numberOfLogLines, resultKeyword, optionSet)
        end
      end
    end
  end

  def execAllRust
    puts "Solving #{@puzzleFilename} with #{@execFilename} (time in seconds)"
    resultMap = Hash[MODE_ARGUMENT_SET.zip(MODE_RESULT_SET)]

    @parallelStrRustSet.each do |parallel|
      @modeStrRustSet.each do |mode|
        numberOfLogLines = mode.empty? ? (NUMBER_OF_LINES + @numberOfPuzzles) : 0
        resultKeyword = resultMap.fetch(mode, nil)
        optionSet = [parallel, mode.split(/\s+/)].flatten()
        desc = ["Rust", @parallelStrRustMap[parallel], @modeStrRustMap[mode]].join(" / ") + " :"
        print desc
        puts exec(numberOfLogLines, resultKeyword, optionSet)
      end
    end
  end

  def makeMap(argSet, descSet)
    maxLength = descSet.map(&:length).max

    newMap = {}
    argSet.zip(descSet).each do |arg, desc|
      newMap[arg] = desc.ljust(maxLength)
    end
    newMap
  end

  def exec(numberOfLogLines, resultKeyword, optionSet)
    Launcher.new(@execFilename, @puzzleFilename, @logFilename, resultKeyword, numberOfLogLines, optionSet).exec
  end
end

puzzleFilename = File.exist?(PRIMARY_DEFAULT_PUZZLE_FILENAME) ?
                   PRIMARY_DEFAULT_PUZZLE_FILENAME : ALT_DEFAULT_PUZZLE_FILENAME
puzzleFilename = ARGV[0] if ARGV.size > 0

if File.exist?(RUST_EXEC_FILENAME)
  ParameterSet.new(puzzleFilename, RUST_EXEC_FILENAME, TEMPORARY_LOGFILE_NAME).execAllRust
end

execFilename   = DEFAULT_EXEC_FILENAME
execFilename   = ARGV[1] if ARGV.size > 1
ParameterSet.new(puzzleFilename, execFilename, TEMPORARY_LOGFILE_NAME).execAll
0
