//! A Sudoku solver written in the Rust programming language
//! # How to Build
//!
//! ```bash
//! $ cd sudoku_rust
//! $ cargo build
//! ```
//!
//! # Solving 9x9 Sudoku puzzles
//! ```bash
//! $ target/{debug|release}/sudoku_rust [options] < sudoku_puzzle.txt
//! $ target/{debug|release}/sudoku_rust [options] [sudoku-puzzle-files]
//! ```
//! The input file __sudoku_puzzle.txt__ is a text file that contains
//! 9x9 sudoku puzzles.  In the text file, each line represents a puzzle
//! and has 81 preset numbers ('1'..'9') or blanks.  See more details in
//! [the author's repository](https://github.com/zettsu-t/sudokusse/blob/master/sudokusse.md).

use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io;
use std::io::BufReader;
use std::io::prelude::*;
use std::sync::atomic::{AtomicBool, Ordering};
extern crate num_cpus;
extern crate crossbeam;
extern crate getopts;

type SudokuCandidate = u64;  // or u32
type SudokuCellGroups = HashMap<usize, Vec<Vec<usize>>>;

const SUDOKU_DIGIT_BASE: u32 = 10;
const SUDOKU_BOX_SIZE: usize = 3;
const SUDOKU_CANDIDATE_SIZE: usize = SUDOKU_GROUP_SIZE;
const SUDOKU_GROUP_SIZE: usize = SUDOKU_BOX_SIZE * SUDOKU_BOX_SIZE;
const SUDOKU_CELL_SIZE: usize = SUDOKU_GROUP_SIZE * SUDOKU_GROUP_SIZE;
const SUDOKU_NO_CANDIDATES: SudokuCandidate = 0;
const SUDOKU_ALL_CANDIDATES: SudokuCandidate = ((1 << SUDOKU_CANDIDATE_SIZE) - 1);

macro_rules! print_if_needed {
    ($cond:expr, $($arg:tt)*) => {if $cond { print!($($arg)*);} }
}

macro_rules! print_unless_silent {
    ($silent:expr, $($arg:tt)*) => { print_if_needed!(!$silent, $($arg)*); }
}

/// Sudoku cell that contains candidates as a bitboard
#[derive(Clone, Copy)]
struct SudokuCell {
    candidates: SudokuCandidate
}

impl SudokuCell {
    fn new() -> SudokuCell {
        SudokuCell { candidates: SUDOKU_NO_CANDIDATES }
    }

    /// Returns a string representation
    fn to_string(&self) -> String {
        let mut line = String::new();
        for candidate in 0..(SUDOKU_CANDIDATE_SIZE as SudokuCandidate) {
            if self.has_candidate(&candidate) {
                line.push_str(&(candidate+1).to_string());
            }
        }
        debug_assert!(line.len() <= SUDOKU_CANDIDATE_SIZE);
        line
    }

    fn preset_full_candidate(&mut self) {
        self.candidates = SUDOKU_ALL_CANDIDATES;
    }

    // Input '1'..'9'
    fn preset_candidate(&mut self, ch: char) {
        let code_digit_base = '1'.to_digit(SUDOKU_DIGIT_BASE).unwrap();
        match ch {
            '1'...'9' => {
                let number = (ch.to_digit(SUDOKU_DIGIT_BASE).unwrap() - code_digit_base) as SudokuCandidate;
                self.candidates = SudokuCell::digit_to_candidate(&number);
            }
            _ => {}
        }
    }

    // Input 0..8
    fn overwrite_candidate(&mut self, candidate: SudokuCandidate) {
        if self.count_candidates() > 1 {
            self.candidates = SudokuCell::digit_to_candidate(&candidate);
            debug_assert!(self.candidates <= SUDOKU_ALL_CANDIDATES);
        }
    }

    fn count_candidates(&self) -> usize {
        let result = (self.candidates & SUDOKU_ALL_CANDIDATES).count_ones() as usize;
        debug_assert!(result <= SUDOKU_CANDIDATE_SIZE);
        result
    }

    fn has_unique_candidate(&self) -> bool {
        SudokuCell::is_unique_candidate(self.candidates)
    }

    fn is_unique_candidate(candidates: SudokuCandidate) -> bool {
        // Check whether only one bit is set
        candidates > 0 && (candidates & (candidates - 1)) == 0
    }

    fn has_all_candidates(&self) -> bool {
        self.candidates == SUDOKU_ALL_CANDIDATES
    }

    // Input 0..8
    fn can_mask(&mut self, candidates: SudokuCandidate) -> bool {
        (self.candidates & SudokuCell::digit_to_candidate(&candidates)) != 0
    }

    fn merge_candidate(&mut self, rhs: &SudokuCell) {
        self.candidates |= rhs.candidates;
        debug_assert!(self.candidates <= SUDOKU_ALL_CANDIDATES);
    }

    // Returns whether no candidates are merged
    fn merge_unique_candidate(&mut self, rhs: &SudokuCell) -> bool {
        debug_assert!(rhs.candidates <= SUDOKU_ALL_CANDIDATES);

        if rhs.has_unique_candidate() {
            if (self.candidates & rhs.candidates) != 0 {
                return true;
            }
            self.candidates |= rhs.candidates;
            debug_assert!(self.candidates <= SUDOKU_ALL_CANDIDATES);
        }
        false
    }

    fn filter_by_candidates(&mut self, rhs: &SudokuCell) {
        debug_assert!(rhs.candidates <= SUDOKU_ALL_CANDIDATES);

        if !self.has_unique_candidate() {
            self.candidates &= !rhs.candidates;
            debug_assert!(self.candidates <= SUDOKU_ALL_CANDIDATES);
        }
    }

    fn fill_unused_candidate(&mut self, rhs: &SudokuCell) {
        debug_assert!(rhs.candidates <= SUDOKU_ALL_CANDIDATES);

        if !self.has_unique_candidate() {
            let candidates = SUDOKU_ALL_CANDIDATES & !rhs.candidates;
            debug_assert!(candidates <= SUDOKU_ALL_CANDIDATES);
            if SudokuCell::is_unique_candidate(candidates) {
                self.candidates = candidates;
            }
        }
    }

    // Input 0..8
    fn digit_to_candidate(candidate: &SudokuCandidate) -> SudokuCandidate {
        debug_assert!(((*candidate) as usize) < SUDOKU_CANDIDATE_SIZE);
        let value = (1 as SudokuCandidate) << candidate;
        debug_assert!(value <= SUDOKU_ALL_CANDIDATES);
        value
    }

    // Input 0..8
    fn has_candidate(&self, candidate: &SudokuCandidate) -> bool {
        debug_assert!(((*candidate) as usize) < SUDOKU_CANDIDATE_SIZE);
        (self.candidates & SudokuCell::digit_to_candidate(&candidate)) != 0
    }
}

#[test]
fn test_sudokucell_new() {
    assert!(SudokuCell::new().candidates == SUDOKU_NO_CANDIDATES);
}

#[test]
fn test_sudokucell_to_string() {
    let test_cases = [(1 as SudokuCandidate, "1"), (2, "2"), (4, "3"), (8, "4"), (16, "5"),
                      (32, "6"), (64, "7"), (128, "8"), (256, "9"),
                      (0b111, "123"), (0b111000, "456"), (0b111000000, "789"),
                      (0x101, "19"), (0xfe, "2345678"), (0x1ff, "123456789")];
    for (candidates, expected) in test_cases.iter() {
        let mut cell = SudokuCell::new();
        cell.candidates = *candidates;
        assert!(cell.to_string() == expected.to_string());
    }
}

#[test]
fn test_sudokucell_preset_full_candidate() {
    let mut cell = SudokuCell::new();
    cell.preset_full_candidate();
    assert!(cell.candidates != SUDOKU_NO_CANDIDATES);
    assert!(cell.candidates == SUDOKU_ALL_CANDIDATES);
}

#[test]
fn test_sudokucell_preset_candidate() {
    let test_cases = [('1', 1 as SudokuCandidate), ('2', 2), ('3', 4), ('4', 8), ('5', 16),
                      ('6', 32), ('7', 64), ('8', 128), ('9', 256)];
    for (ch, expected) in test_cases.iter() {
        let mut cell = SudokuCell::new();
        cell.preset_candidate(*ch);
        assert!(cell.candidates == *expected);
    }
}

#[test]
fn test_sudokucell_preset_non_candidate() {
    let test_cases = ['0', 'a', '.', '_'];
    for ch in test_cases.iter() {
        let mut cell = SudokuCell::new();
        cell.preset_candidate(*ch);
        assert!(cell.candidates == SUDOKU_NO_CANDIDATES);
    }
}

#[test]
fn test_sudokucell_overwrite_candidate() {
    for candidate in 0..(SUDOKU_CANDIDATE_SIZE as SudokuCandidate) {
        let mut cell = SudokuCell::new();
        cell.preset_full_candidate();
        cell.overwrite_candidate(candidate);
        assert!(cell.candidates == (1 << candidate));
    }
}

#[test]
fn test_sudokucell_no_overwrite_candidate() {
    for candidate in 0..(SUDOKU_CANDIDATE_SIZE as SudokuCandidate) {
        let mut cell = SudokuCell::new();
        let current_candidates = 1 << (8 - candidate);
        assert!(current_candidates != candidate);
        cell.candidates = current_candidates;
        cell.overwrite_candidate(candidate);
        assert!(cell.candidates == current_candidates);
    }
}

#[test]
fn test_sudokucell_count_candidates() {
    let test_cases = [(1 as SudokuCandidate, 1 as usize), (256, 1),
                      (0, 0), (0b11, 2), (0b111000000, 3), (0b111_111_111, 9)];
    for (candidates, expected) in test_cases.iter() {
        let mut cell = SudokuCell::new();
        cell.candidates = *candidates;
        assert!(cell.count_candidates() == *expected);
    }
}

#[test]
fn test_sudokucell_has_unique_all_candidate() {
    for candidates in 0..(SUDOKU_ALL_CANDIDATES + 1) {
        let mut cell = SudokuCell::new();
        let expected = candidates.count_ones() == 1;
        assert!(SudokuCell::is_unique_candidate(candidates) == expected);
        cell.candidates = candidates;
        assert!(cell.has_unique_candidate() == expected);
        assert!(cell.has_all_candidates() == (candidates == SUDOKU_ALL_CANDIDATES));
    }
}

#[test]
fn test_sudokucell_can_mask() {
    let test_cases = [(1 as SudokuCandidate, 0 as SudokuCandidate, true),
                      (1, 1, false), (3, 0, true), (3, 1, true), (3, 2, false)];
    for (candidates, other, expected) in test_cases.iter() {
        let mut cell = SudokuCell::new();
        cell.candidates = *candidates;
        assert!(cell.can_mask(*other) == *expected);
    }
}

#[test]
fn test_sudokucell_merge_candidate() {
    let test_cases = [(1 as SudokuCandidate, 1 as SudokuCandidate, 1 as SudokuCandidate),
                      (1, 0b10, 0b11), (0b10, 1, 0b11), (0b11, 1, 0b11), (1, 0b11, 0b11)];
    for (candidates, other, expected) in test_cases.iter() {
        let mut left = SudokuCell::new();
        let mut right = SudokuCell::new();
        left.candidates = *candidates;
        right.candidates = *other;
        left.merge_candidate(&right);
        assert!(left.candidates == *expected);
    }
}

#[test]
fn test_sudokucell_merge_unique_candidate() {
    let test_cases = [(1 as SudokuCandidate, 1 as SudokuCandidate, 1 as SudokuCandidate, true),
                      (1, 0b10, 0b11, false), (0b10, 1, 0b11, false),
                      (0b11, 1, 0b11, true), (1, 0b11, 1, false)];
    for (candidates, other, expected, conflict) in test_cases.iter() {
        let mut left = SudokuCell::new();
        let mut right = SudokuCell::new();
        left.candidates = *candidates;
        right.candidates = *other;
        let actual = left.merge_unique_candidate(&right);
        assert!(left.candidates == *expected);
        assert!(actual == *conflict);
    }
}

#[test]
fn test_sudokucell_filter_by_candidates() {
    let test_cases = [(1 as SudokuCandidate, 1 as SudokuCandidate, 1 as SudokuCandidate),
                      (1, 0b10, 1), (0b10, 0b11, 0b10),
                      (0b11, 1, 0b10), (0b11, 0b110, 1)];
    for (candidates, other, expected) in test_cases.iter() {
        let mut left = SudokuCell::new();
        let mut right = SudokuCell::new();
        left.candidates = *candidates;
        right.candidates = *other;
        left.filter_by_candidates(&right);
        assert!(left.candidates == *expected);
    }
}

#[test]
fn test_sudokucell_fill_unused_candidate() {
    let test_cases = [(1 as SudokuCandidate, 1 as SudokuCandidate, 1 as SudokuCandidate),
                      (1, 0b111_111_101, 1), (0x100, 0b111_111_101, 0x100),
                      (0b11, 0b111_111_101, 0b10), (0x1f0, 0b11_111_111, 0x100),
                      (SUDOKU_ALL_CANDIDATES, !0x100, 0x100),
                      (SUDOKU_ALL_CANDIDATES, !1, 1)];
    for (candidates, other, expected) in test_cases.iter() {
        let mut left = SudokuCell::new();
        let mut right = SudokuCell::new();
        left.candidates = *candidates;
        right.candidates = *other & SUDOKU_ALL_CANDIDATES;
        left.fill_unused_candidate(&right);
        assert!(left.candidates == *expected);
    }
}

#[test]
fn test_sudokucell_digit_to_candidate() {
    let test_cases = [(0, 1 as SudokuCandidate), (1, 2), (2, 4), (3, 8), (4, 16),
                      (5, 32), (6, 64), (7, 128), (8, 256)];
    for (candidate, expected) in test_cases.iter() {
        let actual = SudokuCell::digit_to_candidate(candidate);
        assert!(actual == *expected);
    }
}

#[test]
fn test_sudokucell_has_unique_all_candidatex() {
    for preset_candidates in 0..(SUDOKU_ALL_CANDIDATES + 1) {
        let mut cell = SudokuCell::new();
        cell.candidates = preset_candidates;
        for candidate in 0..(SUDOKU_CANDIDATE_SIZE as SudokuCandidate) {
            let expected = preset_candidates & (1 << candidate) != 0;
            let actual = cell.has_candidate(&candidate);
            assert!(actual == expected);
        }
   }
}

/// Generator for sudoku cellgroups
struct SudokuGroupGen {
    index: usize
}

impl SudokuGroupGen {
    fn new() -> SudokuGroupGen {
        SudokuGroupGen { index:0 }
    }
}

impl Iterator for SudokuGroupGen {
    type Item = Vec<usize>;

    fn next(&mut self) -> Option<Vec<usize>> {
        if self.index < SUDOKU_GROUP_SIZE {
            let lower = self.index * SUDOKU_GROUP_SIZE;
            let upper = (self.index + 1) * SUDOKU_GROUP_SIZE;
            self.index += 1;
            Some((lower..upper).collect::<Vec<_>>())
        } else if self.index < (SUDOKU_GROUP_SIZE * 2) {
            let column = self.index - SUDOKU_GROUP_SIZE;
            self.index += 1;
            Some((0..SUDOKU_GROUP_SIZE).map(
                |row| column + row * SUDOKU_GROUP_SIZE).collect::<Vec<_>>())
        } else if self.index < (SUDOKU_GROUP_SIZE * 3) {
            let box_index = self.index - SUDOKU_GROUP_SIZE * 2;
            let offset = (box_index % 3) * 3  + (box_index / 3) * SUDOKU_GROUP_SIZE * 3;
            let mut indexes : Vec<usize> = Vec::new();
            for row in 0..SUDOKU_BOX_SIZE {
                let x = (0..SUDOKU_BOX_SIZE).map(
                    |column| row * SUDOKU_GROUP_SIZE + column + offset).collect::<Vec<_>>();
                indexes.extend(x);
            }
            self.index += 1;
            Some(indexes)
        } else {
            None
        }
    }
}

#[test]
fn test_sudokugroupgen_next() {
    let number_of_group_type: usize = 3;
    let mut group = SudokuGroupGen::new();
    let mut count = 0;

    for i in 0..(SUDOKU_GROUP_SIZE * number_of_group_type) {
        let mut expected: Vec<usize> = Vec::new();
        match i {
            0 => {expected.extend([0, 1, 2, 3, 4, 5, 6, 7, 8].to_vec()); () },
            8 => {expected.extend([72, 73, 74, 75, 76, 77, 78, 79, 80].to_vec()); () },
            9 => {expected.extend([0, 9, 18, 27, 36, 45, 54, 63, 72].to_vec()); () },
            17 => {expected.extend([8, 17, 26, 35, 44, 53, 62, 71, 80].to_vec()); () },
            23 => {expected.extend([33, 34, 35, 42, 43, 44, 51, 52, 53].to_vec()); () },
            _ => {}
        }

        let actual = group.next().unwrap();
        if !expected.is_empty() {
            assert!(actual == expected);
            count += 1;
        }
    }

    assert!(count == 5);
    assert!(group.next() == None);
}

/// Sudoku cell map
struct SudokuMap<'a> {
    cells : Vec<SudokuCell>,
    groups : &'a SudokuCellGroups,
}

impl<'a> SudokuMap<'a> {
    fn new(line: &str, cell_groups: &'a SudokuCellGroups) -> SudokuMap<'a> {
        let mut cells = vec![SudokuCell::new(); SUDOKU_CELL_SIZE];
        for cell in &mut cells {
            cell.preset_full_candidate();
        }

        // Leave tail cells if the line is shorter than the number of cells.
        for (index, ch) in line.chars().enumerate() {
            cells[index].preset_candidate(ch);
        };
        SudokuMap { cells:cells, groups: cell_groups }
    }

    fn to_string(&self, one_line: bool) -> String {
        let mut full_text = String::new();

        for row in 0..SUDOKU_GROUP_SIZE {
            let mut line = String::new();
            for column in 0..SUDOKU_GROUP_SIZE {
                let index = row * SUDOKU_GROUP_SIZE + column;
                let cell_str = self.cells[index].to_string();
                line.push_str(&cell_str);
                if !one_line {
                    line.push_str(":");
                }
            }
            if !one_line {
                line.push_str("\n");
            }
            full_text.push_str(&line);
        }
        full_text
    }

    fn solve(&mut self) -> bool {
        let mut prev_count = SUDOKU_CELL_SIZE * SUDOKU_CANDIDATE_SIZE;
        loop {
            self.filter_unique_candidates();
            self.find_unused_candidates();

            if self.is_solved() {
                return true;
            }

            if !self.is_consistent() {
                return false;
            }

            let count = (0..SUDOKU_CELL_SIZE).fold(
                0, |sum, index| sum + self.cells[index].count_candidates());
            if prev_count == count {
                break;
            }
            prev_count = count;
        }

        let index_min_count = self.find_backtracking_target();
        if self.cells[index_min_count].count_candidates() <= 1 {
            return false;
        }

        for candidate in 0..(SUDOKU_CANDIDATE_SIZE as SudokuCandidate) {
            if self.cells[index_min_count].can_mask(candidate) {
                let mut new_map = SudokuMap::new("", self.groups);
                new_map.cells = self.cells.clone();
                new_map.cells[index_min_count].overwrite_candidate(candidate);
                if new_map.solve() {
                    self.cells = new_map.cells;
                    return self.is_solved();
                }
            }
        }

        false
    }

    fn filter_unique_candidates(&mut self) {
        for target_index in 0..SUDOKU_CELL_SIZE {
            if self.cells[target_index].has_unique_candidate() {
                continue;
            }

            let mut sum = SudokuCell::new();
            for group in self.groups[&target_index].iter() {
                for cell_index in group.iter() {
                    if *cell_index != target_index {
                        sum.merge_unique_candidate(&self.cells[*cell_index]);
                    }
                }
            }
            self.cells[target_index].filter_by_candidates(&sum);
        }
    }

    fn find_unused_candidates(&mut self) {
        for target_index in 0..SUDOKU_CELL_SIZE {
            if self.cells[target_index].has_unique_candidate() {
                continue;
            }

            for group in self.groups[&target_index].iter() {
                let mut sum = SudokuCell::new();
                for cell_index in group.iter() {
                    if *cell_index != target_index {
                        sum.merge_candidate(&self.cells[*cell_index]);
                    }
                }
                self.cells[target_index].fill_unused_candidate(&sum);
            }
        }
    }

    fn find_backtracking_target(&self) -> usize {
        // SUDOKU_CELL_SIZE = a large enough number
        let mut min_count = SUDOKU_CELL_SIZE;
        let mut cell_counts = Vec::new();

        for row_index in 0..SUDOKU_GROUP_SIZE {
            let mut row_counts = vec![SUDOKU_CELL_SIZE; SUDOKU_GROUP_SIZE];
            for column_index in 0..SUDOKU_GROUP_SIZE {
                let cell_count = self.cells[row_index * SUDOKU_GROUP_SIZE + column_index].count_candidates();
                if cell_count > 1 {
                    row_counts[column_index] = cell_count;
                    if cell_count < min_count {
                        min_count = cell_count;
                    }
                }
            }
            cell_counts.push(row_counts);
        }

        let row_counts = cell_counts.iter().map(
            |row| row.iter().fold(
                0, |sum, &count| sum + if count == min_count { 1 } else { 0 })).
            collect::<Vec<usize>>();

        let max_row_count = row_counts.iter().max().unwrap();
        let target_row = row_counts.iter().position(|&count| count == *max_row_count).unwrap();
        let target_column = cell_counts[target_row].iter().position(
            |&count| count == min_count).unwrap();

        target_row * SUDOKU_GROUP_SIZE + target_column
    }

    fn is_solved(&self) -> bool {
        for cell_index in 0..SUDOKU_CELL_SIZE {
            if !self.cells[cell_index].has_unique_candidate() {
                return false;
            }
        }

        let mut group_gen = SudokuGroupGen::new();
        while let Some(indexes) = group_gen.next() {
            if !self.is_group_solved(indexes) {
                return false;
            }
        }
        true
    }

    fn is_group_solved(&self, cell_indexes: Vec<usize>) -> bool {
        let mut sum = SudokuCell::new();
        let mut conflict = false;
        for cell_index in cell_indexes {
            let cell = &self.cells[cell_index];
            conflict |= sum.merge_unique_candidate(&cell);
        }
        sum.has_all_candidates() && !conflict
    }

    fn is_consistent(&self) -> bool {
        let mut group_gen = SudokuGroupGen::new();
        while let Some(indexes) = group_gen.next() {
            if !self.is_group_consistent(indexes) {
                return false;
            }
        }
        true
    }

    fn is_group_consistent(&self, cell_indexes: Vec<usize>) -> bool {
        let mut all_candidates = SudokuCell::new();
        let mut unique_candidates = SudokuCell::new();
        let mut conflict = false;

        for cell_index in cell_indexes {
            all_candidates.merge_candidate(&self.cells[cell_index]);
            conflict |= unique_candidates.merge_unique_candidate(&self.cells[cell_index]);
            if conflict {
                return false;
            }
        }
        all_candidates.has_all_candidates() && !conflict
    }
}

// Set of sudoku cellgroups
fn create_group_index_set() -> SudokuCellGroups {
    let mut groups = HashMap::new();
    for cell_index in 0..SUDOKU_CELL_SIZE {
        let mut group_candidates = Vec::new();
        let row_number = cell_index / SUDOKU_GROUP_SIZE;
        let row_lower = row_number * SUDOKU_GROUP_SIZE;
        let row_upper = (row_number + 1) * SUDOKU_GROUP_SIZE;
        let row = (row_lower..row_upper).collect::<Vec<_>>();
        group_candidates.push(row);

        let column_number = cell_index % SUDOKU_GROUP_SIZE;
        let column = (0..SUDOKU_GROUP_SIZE).map(|i| column_number + i * SUDOKU_GROUP_SIZE).collect::<Vec<_>>();
        group_candidates.push(column);

        let box_offset = (row_number / SUDOKU_BOX_SIZE) * SUDOKU_BOX_SIZE * SUDOKU_GROUP_SIZE +
            (column_number / SUDOKU_BOX_SIZE) * SUDOKU_BOX_SIZE;
        let mut cell_box = Vec::new();
        for row_offset in 0..SUDOKU_BOX_SIZE {
            let x = (0..SUDOKU_BOX_SIZE).map(|column_offset| row_offset * SUDOKU_GROUP_SIZE + column_offset + box_offset);
            cell_box.extend(x);
        }
        group_candidates.push(cell_box);
        groups.insert(cell_index, group_candidates);
    }
    groups
}

fn exec_threads(lines: &Vec<String>) -> (bool, String) {
    // Shares cell groups between all questions
    let count = lines.len();
    let n_cores = std::cmp::min(num_cpus::get(), count);
    let lines_per_core = count / n_cores;
    let mut children = vec![];
    let total_result = AtomicBool::new(true);
    let mut all_answers = String::new();

    crossbeam::scope(|scope| {
        for core_index in 0..n_cores {
            let cell_groups = create_group_index_set();
            let start_pos = core_index * lines_per_core;
            let end_pos = if (core_index + 1) == n_cores {
                count
            } else {
                start_pos + lines_per_core
            };

            children.push(scope.spawn(move || {
                let mut result = true;
                let mut answers = String::new();
                for line_index in start_pos..end_pos {
                    let mut sudoku_map = SudokuMap::new(&lines[line_index], &cell_groups);
                    result &= sudoku_map.solve();
                    let s = sudoku_map.to_string(true);
                    answers.push_str(&s);
                    answers.push_str("\n");
                }
                (result, answers)
            }))
        }

        for child in children {
            let (result, answers) = child.join().unwrap() as (bool, String);
            total_result.fetch_and(result, Ordering::SeqCst);
            all_answers.push_str(&answers);
        }
    });

    return (total_result.load(Ordering::SeqCst), all_answers);
}

fn exec_single_threads(lines: &Vec<String>) -> (bool, String) {
    let cell_groups = create_group_index_set();
    let mut result = true;
    let mut answers = String::new();

    for line in lines {
        let mut sudoku_map = SudokuMap::new(&line, &cell_groups);
        result &= sudoku_map.solve();
        answers.push_str(&sudoku_map.to_string(true));
        answers.push_str("\n");
    }

    return (result, answers);
}

fn main() {
    // cited from the getopts example
    // https://docs.rs/getopts/0.2.18/getopts/
    let args: Vec<String> = env::args().collect();
    let program = args[0].clone();
    let mut opts = getopts::Options::new();
    opts.optopt("m", "max_size", "Solve only the head NUMBER of puzzles", "NUMBER");
    opts.optflag("s", "silent", "Do not print solutions");
    opts.optflag("1", "single_threaded", "Run single-threaded");
    opts.optflag("h", "help", "Print this help menu");

    let matches = match opts.parse(&args[1..]) {
        Ok(m) => { m }
        Err(f) => { panic!(f.to_string()) }
    };

    if matches.opt_present("h") {
        let brief = format!("Usage: {} FILE(s) [options]", program);
        print!("{}", opts.usage(&brief));
        return;
    }

    let max_count = match &matches.opt_str("m") {
        Some(num) => Some(num.parse().unwrap()),
        None => None,
    };

    let silent = matches.opt_present("s");

    let mut lines = Vec::new();
    let mut count : usize = 0;
    if matches.free.is_empty() {
        let stdin = io::stdin();
        for line in stdin.lock().lines() {
            let line = line.unwrap();
            lines.push(line);
            count += 1;
            if max_count != None && count >= max_count.unwrap() {
                break;
            }
       }
    } else {
        for filename in &matches.free {
            let infile = File::open(filename).unwrap();
            for line in BufReader::new(infile).lines() {
                let line = line.unwrap();
                lines.push(line);
                count += 1;
                if max_count != None && count >= max_count.unwrap() {
                    break;
                }
            }
        }
    }

    let (result, answers) = if matches.opt_present("1") {
        print_unless_silent!(silent, "Solving in Rust (single-threaded)\n");
        exec_single_threads(&lines)
    } else {
        print_unless_silent!(silent, "Solving in Rust\n");
        exec_threads(&lines)
    };

    print_unless_silent!(silent, "{}", answers);
    print_if_needed!(result && !silent, "All {} cases passed.\n", count);
    if !result {
        std::process::exit(1);
    }
}
