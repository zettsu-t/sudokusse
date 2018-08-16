use std::collections::HashMap;
use std::io;
use std::io::prelude::*;
use std::sync::atomic::{AtomicBool, Ordering};
extern crate crossbeam;

type SudokuCandidate = u64;  // or u32
type SudokuCellGroups = HashMap<usize, Vec<Vec<usize>>>;

const SUDOKU_MAX_THREADS: usize = 4;
const SUDOKU_DIGIT_BASE: u32 = 10;
const SUDOKU_BOX_SIZE: usize = 3;
const SUDOKU_NO_CANDIDATES: SudokuCandidate = 0;
const SUDOKU_ALL_CANDIDATES: SudokuCandidate = ((1 << SUDOKU_CANDIDATE_SIZE) - 1);

const SUDOKU_GROUP_SIZE: usize = SUDOKU_BOX_SIZE * SUDOKU_BOX_SIZE;
const SUDOKU_CANDIDATE_SIZE: usize = SUDOKU_GROUP_SIZE;
const SUDOKU_CELL_SIZE: usize = SUDOKU_GROUP_SIZE * SUDOKU_GROUP_SIZE;

// Sudoku cell that contains candidates as a bitboard
#[derive(Clone, Copy)]
struct SudokuCell {
    candidates: SudokuCandidate
}

impl SudokuCell {
    fn new() -> SudokuCell {
        SudokuCell { candidates: SUDOKU_NO_CANDIDATES }
    }

    fn to_string(&self) -> String {
        let mut line = String::new();
        for candidate in 0..(SUDOKU_CANDIDATE_SIZE as SudokuCandidate) {
            if self.has_candidate(&candidate) {
                line.push_str(&(candidate+1).to_string());
            }
        }
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
                self.candidates = self.digit_to_candidate(&number);
            }
            _ => {}
        }
    }

    // Input 0..8
    fn overwrite_candidate(&mut self, candidate: SudokuCandidate) {
        if self.count_candidates() > 1 {
            self.candidates = self.digit_to_candidate(&candidate);
        }
    }

    fn count_candidates(&self) -> usize {
        (self.candidates & SUDOKU_ALL_CANDIDATES).count_ones() as usize
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
        (self.candidates & self.digit_to_candidate(&candidates)) != 0
    }

    fn merge_candidate(&mut self, rhs: &SudokuCell) {
        self.candidates |= rhs.candidates;
    }

    // Returns whether no candidates are merged
    fn merge_unique_candidate(&mut self, rhs: &SudokuCell) -> bool {
        if rhs.has_unique_candidate() {
            if (self.candidates & rhs.candidates) != 0 {
                return true;
            }
            self.candidates |= rhs.candidates;
        }
        false
    }

    fn filter_by_candidates(&mut self, rhs: &SudokuCell) {
        if !self.has_unique_candidate() {
            self.candidates &= !rhs.candidates;
        }
    }

    fn fill_unused_candidate(&mut self, rhs: &SudokuCell) {
        if !self.has_unique_candidate() {
            let candidates = SUDOKU_ALL_CANDIDATES & !rhs.candidates;
            if SudokuCell::is_unique_candidate(candidates) {
                self.candidates = candidates;
            }
        }
    }

    // Input 0..8
    fn digit_to_candidate(&self, candidate: &SudokuCandidate) -> SudokuCandidate {
        (1 as SudokuCandidate) << candidate
    }

    // Input 0..8
    fn has_candidate(&self, candidate: &SudokuCandidate) -> bool {
        (self.candidates & self.digit_to_candidate(&candidate)) != 0
    }
}

// Generator for sudoku cellgroups
struct SudokuGroupGen {
    index: usize
}

impl SudokuGroupGen {
    fn new() -> SudokuGroupGen {
        SudokuGroupGen { index:0 }
    }

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

// Sudoku cell map
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

fn exec_threads(count:usize, lines: &Vec<String>) {
    // Shares cell groups between all questions
    let n_cores = std::cmp::min(SUDOKU_MAX_THREADS, count);
    let lines_per_core = count / n_cores;
    let mut children = vec![];
    let total_result = AtomicBool::new(true);

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
            print!("{}", answers);
        }
    });

    if total_result.load(Ordering::SeqCst) {
        println!("All {} cases passed.", count);
    }
}

fn main() {
    println!("Solving in Rust");
    let stdin = io::stdin();
    let mut lines = Vec::new();
    let mut count : usize = 0;
    for line in stdin.lock().lines() {
        let line = line.unwrap();
        lines.push(line);
        count += 1;
    }
    exec_threads(count, &lines);
}
