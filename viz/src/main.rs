mod app;
mod data;
mod ui;

use app::App;
use crossterm::{
    ExecutableCommand,
    event::{self, Event, KeyCode, KeyEventKind},
    terminal::{EnterAlternateScreen, LeaveAlternateScreen, disable_raw_mode, enable_raw_mode},
};
use ratatui::prelude::*;
use std::{error::Error, io::stdout};

fn main() -> Result<(), Box<dyn Error>> {
    // Setup terminal
    stdout().execute(EnterAlternateScreen)?;
    enable_raw_mode()?;
    let mut terminal = Terminal::new(CrosstermBackend::new(stdout()))?;
    terminal.clear()?;

    // Load data from the parent directory
    let mut app = App::new("../heap_history.jsonl", "../results.json");

    loop {
        terminal.draw(|frame| ui::ui(frame, &app))?;

        if event::poll(std::time::Duration::from_millis(100))? {
            if let Event::Key(key) = event::read()? {
                if key.kind == KeyEventKind::Press {
                    match key.code {
                        KeyCode::Char('q') | KeyCode::Esc => break,
                        KeyCode::Right | KeyCode::Char('n') => app.next_step(),
                        KeyCode::Left | KeyCode::Char('p') => app.prev_step(),
                        KeyCode::Tab => app.toggle_tab(),
                        _ => {}
                    }
                }
            }
        }
    }

    // Restore terminal
    disable_raw_mode()?;
    stdout().execute(LeaveAlternateScreen)?;
    Ok(())
}
