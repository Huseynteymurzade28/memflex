use crossterm::{
    event::{self, KeyCode, KeyEventKind},
    terminal::{disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
    ExecutableCommand,
};
use ratatui::{
    prelude::*,
    widgets::{BarChart, Block, Borders},
};
use serde::Deserialize;
use std::{error::Error, fs, io::stdout};

#[derive(Deserialize, Debug)]
struct BenchmarkResult {
    name: String,
    time: f64,
    total_blocks: u64,
}

fn main() -> Result<(), Box<dyn Error>> {
    // Read data
    let data = fs::read_to_string("../results.json").expect("Unable to read results.json");
    let results: Vec<BenchmarkResult> = serde_json::from_str(&data)?;

    // Setup terminal
    stdout().execute(EnterAlternateScreen)?;
    enable_raw_mode()?;
    let mut terminal = Terminal::new(CrosstermBackend::new(stdout()))?;
    terminal.clear()?;

    loop {
        terminal.draw(|frame| {
            let layout = Layout::default()
                .direction(Direction::Vertical)
                .constraints([Constraint::Percentage(50), Constraint::Percentage(50)])
                .split(frame.area());

            // Time Chart
            let time_data: Vec<(&str, u64)> = results
                .iter()
                .map(|r| (r.name.as_str(), (r.time * 1000.0) as u64)) // Convert to ms
                .collect();

            let time_chart = BarChart::default()
                .block(Block::default().title("Execution Time (ms)").borders(Borders::ALL))
                .data(&time_data)
                .bar_width(10)
                .bar_gap(5)
                .value_style(Style::default().fg(Color::Black).bg(Color::White))
                .style(Style::default().fg(Color::Cyan));

            frame.render_widget(time_chart, layout[0]);

            // Fragmentation Chart
            let frag_data: Vec<(&str, u64)> = results
                .iter()
                .map(|r| (r.name.as_str(), r.total_blocks))
                .collect();

            let frag_chart = BarChart::default()
                .block(Block::default().title("Total Block Count").borders(Borders::ALL))
                .data(&frag_data)
                .bar_width(10)
                .bar_gap(5)
                .value_style(Style::default().fg(Color::Black).bg(Color::White))
                .style(Style::default().fg(Color::Magenta));

            frame.render_widget(frag_chart, layout[1]);
        })?;

        if event::poll(std::time::Duration::from_millis(100))? {
            if let event::Event::Key(key) = event::read()? {
                if key.kind == KeyEventKind::Press && key.code == KeyCode::Char('q') {
                    break;
                }
            }
        }
    }

    stdout().execute(LeaveAlternateScreen)?;
    disable_raw_mode()?;
    Ok(())
}
