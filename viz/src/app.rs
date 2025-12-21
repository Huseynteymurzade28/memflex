use crate::data::{BenchmarkResult, HeapStep};
use std::fs::{self, File};
use std::io::{BufRead, BufReader};

pub enum CurrentTab {
    Visualizer,
    Statistics,
}

pub struct App {
    pub steps: Vec<HeapStep>,
    pub current_step_index: usize,
    pub benchmark_results: Vec<BenchmarkResult>,
    pub current_tab: CurrentTab,
}

impl App {
    pub fn new(history_path: &str, results_path: &str) -> Self {
        let file = File::open(history_path).expect("Unable to open history file");
        let reader = BufReader::new(file);
        let steps: Vec<HeapStep> = reader
            .lines()
            .map(|line| serde_json::from_str(&line.unwrap()).unwrap())
            .collect();

        let results_data = fs::read_to_string(results_path).unwrap_or_else(|_| "[]".to_string());
        let benchmark_results: Vec<BenchmarkResult> =
            serde_json::from_str(&results_data).unwrap_or_default();

        Self {
            steps,
            current_step_index: 0,
            benchmark_results,
            current_tab: CurrentTab::Visualizer,
        }
    }

    pub fn next_step(&mut self) {
        if self.current_step_index < self.steps.len() - 1 {
            self.current_step_index += 1;
        }
    }

    pub fn prev_step(&mut self) {
        if self.current_step_index > 0 {
            self.current_step_index -= 1;
        }
    }

    pub fn current_step(&self) -> &HeapStep {
        &self.steps[self.current_step_index]
    }

    pub fn toggle_tab(&mut self) {
        self.current_tab = match self.current_tab {
            CurrentTab::Visualizer => CurrentTab::Statistics,
            CurrentTab::Statistics => CurrentTab::Visualizer,
        };
    }
}
