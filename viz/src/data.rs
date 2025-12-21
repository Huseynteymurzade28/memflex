use serde::Deserialize;

#[derive(Deserialize, Debug, Clone)]
pub struct BlockState {
    pub addr: String,
    pub size: usize,
    pub is_free: bool,
}

#[derive(Deserialize, Debug, Clone)]
pub struct HeapStep {
    pub step: usize,
    pub algo: String,
    pub op: String,
    pub highlight: String,
    pub blocks: Vec<BlockState>,
}

#[derive(Deserialize, Debug, Clone)]
pub struct BenchmarkResult {
    pub name: String,
    pub time: f64,
    pub total_blocks: u64,
}
