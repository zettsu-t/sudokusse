use std::io;
use std::io::prelude::*;

type SudokuCandidate = u64;
const SUDOKU_DIGIT_BASE: u32 = 10;
const SUDOKU_CANDIDATE_SIZE: usize = 9;
const SUDOKU_BOX_SIZE: usize = 3;
const SUDOKU_GROUP_SIZE: usize = 9;
const SUDOKU_CELL_SIZE: usize = 81;
const SUDOKU_ALL_CANDIDATES: SudokuCandidate = ((1 << SUDOKU_CANDIDATE_SIZE) - 1);

#[derive(Clone, Copy)]
struct SudokuCell {
    candidates: SudokuCandidate
}

impl SudokuCell {
    fn new() -> SudokuCell {
        SudokuCell { candidates: SUDOKU_ALL_CANDIDATES }
    }

    // '1'..'9'
    fn preset_candidate(&mut self, ch: char) {
        let code_digit_base = '1'.to_digit(SUDOKU_DIGIT_BASE).unwrap();
        match ch {
            '1'...'9' => {
                let number = (ch.to_digit(SUDOKU_DIGIT_BASE).unwrap() - code_digit_base) as SudokuCandidate;
                self.candidates = self.digit_to_candidate(&number);
                ()
            }
            _ => {()}
        }
    }

    fn digit_to_candidate(&self, candidate: &SudokuCandidate) -> SudokuCandidate {
        (1 as SudokuCandidate) << candidate
    }

    fn has_candidate(&self, candidate: &SudokuCandidate) -> bool {
        (self.candidates & self.digit_to_candidate(&candidate)) != 0
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

    fn clear_candidate(&mut self) {
        self.candidates = 0
    }

    fn has_unique_candidate(&self) -> bool {
        (self.candidates & (self.candidates - 1)) == 0
    }

    fn can_merge(&mut self, candidates: SudokuCandidate) -> bool {
        (self.candidates & self.digit_to_candidate(&candidates)) != 0
    }

    fn merge_unique_candidate(&mut self, rhs: &SudokuCell) -> bool {
        if rhs.has_unique_candidate() {
            if (self.candidates & rhs.candidates) != 0 {
                return true;
            }
            self.candidates |= rhs.candidates;
        }
        false
    }

    fn merge_candidate(&mut self, rhs: &SudokuCell) {
        self.candidates |= rhs.candidates;
    }

    fn mask_candidate(&mut self, rhs: &SudokuCell) {
        if rhs.count_candidates() == 1 && (self.candidates & rhs.candidates) != 0 {
            self.candidates -= rhs.candidates;
        }
    }

    fn has_all_candidates(&self) -> bool {
        self.candidates == SUDOKU_ALL_CANDIDATES
    }

    fn overwrite_candidate(&mut self, candidate: SudokuCandidate) {
        if self.count_candidates() > 1 {
            self.candidates = self.digit_to_candidate(&candidate);
        }
    }

    fn count_candidates(&self) -> usize {
        let mut count = 0;
        for candidate in 0..(SUDOKU_CANDIDATE_SIZE as SudokuCandidate) {
            if (self.candidates & self.digit_to_candidate(&candidate)) != 0 {
                count += 1;
            }
        }
        count
    }
}

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
            Some((0..SUDOKU_GROUP_SIZE).map(|i| column + i * SUDOKU_GROUP_SIZE).collect::<Vec<_>>())
        } else if self.index < (SUDOKU_GROUP_SIZE * 3) {
            let box_index = self.index - SUDOKU_GROUP_SIZE * 2;
            let offset = (box_index % 3) * 3  + (box_index / 3) * SUDOKU_GROUP_SIZE * 3;
            let mut indexes : Vec<usize> = Vec::new();
            for row in 0..SUDOKU_BOX_SIZE {
                let x = (0..SUDOKU_BOX_SIZE).map(|column| row * SUDOKU_GROUP_SIZE + column + offset).collect::<Vec<_>>();
                indexes.extend(x);
            }
            self.index += 1;
            Some(indexes)
        } else {
            None
        }
    }
}

struct SudokuMap {
    cells : Vec<SudokuCell>
}

impl SudokuMap {
    fn new(line: &str) -> SudokuMap {
        let mut cells = vec![SudokuCell::new(); SUDOKU_CELL_SIZE];
        for (index, ch) in line.chars().enumerate() {
            cells[index].preset_candidate(ch);
        }
        SudokuMap { cells:cells }
    }

    fn to_string(&self) -> String {
        let mut full_text = String::new();

        for row in 0..SUDOKU_GROUP_SIZE {
            let mut line = String::new();
            for column in 0..SUDOKU_GROUP_SIZE {
                let index = row * SUDOKU_GROUP_SIZE + column;
                let cell_str = self.cells[index].to_string();
                line.push_str(&cell_str);
                line.push_str(":");
            }
            line.push_str("\n");
            full_text.push_str(&line);
        }
        full_text
    }

    fn solve(&mut self) -> bool {
        self.solve_from(0)
    }

    fn filter_unique_candidates(&mut self) {
        let mut group_gen = SudokuGroupGen::new();
        while let Some(indexes) = group_gen.next() {
            let froms = indexes.clone();
            for index in froms {
                let cell = self.cells[index];
                let targets = indexes.clone();
                for target in targets {
                    if target != index {
                        self.cells[target].mask_candidate(&cell);
                    }
                }
            }
        }
    }

    fn solve_from(&mut self, start_pos : usize) -> bool {
        self.filter_unique_candidates();
        if !self.is_consistent() {
            return false;
        }

        for i in start_pos..SUDOKU_CELL_SIZE {
            let count = self.cells[i].count_candidates();
            if count == 0 {
                return false;
            } else if count == 1 {
                continue;
            }
            for candidate in 0..(SUDOKU_CANDIDATE_SIZE as SudokuCandidate) {
                let mut target_cell = self.cells[i].clone();
                if target_cell.can_merge(candidate) {
                    let mut new_map = SudokuMap::new("");
                    new_map.cells = self.cells.clone();
                    new_map.cells[i].overwrite_candidate(candidate);
                    if new_map.solve_from(start_pos + 1) {
                        self.cells = new_map.cells;
                        return true;
                    }
                }
            }
        }

        self.is_solved()
    }

    fn is_solved(&self) -> bool {
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
        sum.clear_candidate();
        for cell_index in cell_indexes {
            conflict |= sum.merge_unique_candidate(&self.cells[cell_index]);
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
        all_candidates.clear_candidate();
        unique_candidates.clear_candidate();

        for cell_index in cell_indexes {
            all_candidates.merge_candidate(&self.cells[cell_index]);
            conflict |= unique_candidates.merge_unique_candidate(&self.cells[cell_index]);
        }
        all_candidates.has_all_candidates() && !conflict
    }
}

fn main() {
    let stdin = io::stdin();

    for line in stdin.lock().lines() {
        let line = line.unwrap();
        let mut sudoku_map = SudokuMap::new(&line);
        let result = sudoku_map.solve();
        let answer = sudoku_map.to_string();
        if result {
            println!("Solved\n{}", answer);
        } else {
            println!("Not solved\n{}", answer);
        }
    }
}
