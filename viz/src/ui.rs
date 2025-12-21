use crate::app::{App, CurrentTab};
use ratatui::{
    prelude::*,
    widgets::{BarChart, Block, Borders, Paragraph, Tabs},
};

pub fn ui(frame: &mut Frame, app: &App) {
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([
            Constraint::Length(3), // Tabs
            Constraint::Min(1),    // Content
            Constraint::Length(3), // Footer
        ])
        .split(frame.area());

    // Tabs
    let titles = vec!["Visualizer", "Statistics"];
    let tab_index = match app.current_tab {
        CurrentTab::Visualizer => 0,
        CurrentTab::Statistics => 1,
    };

    let tabs = Tabs::new(titles)
        .block(Block::default().borders(Borders::ALL).title("MemFlex Viz"))
        .select(tab_index)
        .highlight_style(
            Style::default()
                .fg(Color::Yellow)
                .add_modifier(Modifier::BOLD),
        );
    frame.render_widget(tabs, chunks[0]);

    // Content
    match app.current_tab {
        CurrentTab::Visualizer => render_visualizer(frame, app, chunks[1]),
        CurrentTab::Statistics => render_statistics(frame, app, chunks[1]),
    }

    // Footer
    let footer_text = match app.current_tab {
        CurrentTab::Visualizer => {
            "Controls: [n] Next Step | [p] Prev Step | [TAB] Switch View | [q] Quit"
        }
        CurrentTab::Statistics => "Controls: [TAB] Switch View | [q] Quit",
    };
    let footer = Paragraph::new(footer_text)
        .block(Block::default().borders(Borders::ALL).title("Status"))
        .alignment(Alignment::Center);
    frame.render_widget(footer, chunks[2]);
}

fn render_visualizer(frame: &mut Frame, app: &App, area: Rect) {
    if app.steps.is_empty() {
        return;
    }
    let step = app.current_step();

    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([
            Constraint::Length(3), // Info Header
            Constraint::Min(1),    // Memory Map
            Constraint::Length(3), // Legend
        ])
        .split(area);

    // Info Header
    let header_text = format!(
        "Algorithm: {} | Step: {} | Op: {} | Highlight: {}",
        step.algo, step.step, step.op, step.highlight
    );
    let header = Paragraph::new(header_text)
        .block(
            Block::default()
                .borders(Borders::ALL)
                .title("Current Operation"),
        )
        .style(Style::default().fg(Color::Cyan));
    frame.render_widget(header, chunks[0]);

    // Memory Map
    let map_area = chunks[1];
    let width = map_area.width as usize;

    let total_size: usize = step.blocks.iter().map(|b| b.size).sum();

    if total_size > 0 {
        let mut x_offset = map_area.x;
        let y_offset = map_area.y + 2;
        let bar_height = 5;

        for block in &step.blocks {
            let mut block_width =
                (block.size as f64 / total_size as f64 * width as f64).round() as u16;
            block_width = block_width.max(1);

            if x_offset + block_width > map_area.x + map_area.width {
                block_width = (map_area.x + map_area.width).saturating_sub(x_offset);
            }
            if block_width == 0 {
                break;
            }

            let rect = Rect::new(x_offset, y_offset, block_width, bar_height);

            let color = if block.is_free {
                Color::Green
            } else {
                Color::Red
            };

            let is_highlighted = block.addr == step.highlight;
            let style = if is_highlighted {
                Style::default()
                    .bg(color)
                    .fg(Color::White)
                    .add_modifier(Modifier::BOLD)
            } else {
                Style::default().bg(color).fg(Color::Black)
            };

            let block_widget = Block::default().borders(Borders::ALL).style(style);

            frame.render_widget(block_widget, rect);

            if block_width > 4 {
                let label = format!("{}", block.size);
                let label_widget = Paragraph::new(label)
                    .style(Style::default().fg(if is_highlighted {
                        Color::White
                    } else {
                        Color::Black
                    }))
                    .alignment(Alignment::Center);

                let label_rect = Rect::new(x_offset + 1, y_offset + 2, block_width - 2, 1);
                frame.render_widget(label_widget, label_rect);
            }

            x_offset += block_width;
        }
    }

    // Legend
    let legend_text = vec![
        Span::styled(
            " Used (Red) ",
            Style::default().bg(Color::Red).fg(Color::Black),
        ),
        Span::raw(" "),
        Span::styled(
            " Free (Green) ",
            Style::default().bg(Color::Green).fg(Color::Black),
        ),
        Span::raw(" "),
        Span::styled(
            " Highlighted (Bold) ",
            Style::default().add_modifier(Modifier::BOLD),
        ),
        Span::raw(format!(
            " | Total Size: {} bytes | Blocks: {}",
            total_size,
            step.blocks.len()
        )),
    ];
    let legend = Paragraph::new(Line::from(legend_text))
        .block(Block::default().borders(Borders::ALL).title("Legend"))
        .alignment(Alignment::Center);
    frame.render_widget(legend, chunks[2]);
}

fn render_statistics(frame: &mut Frame, app: &App, area: Rect) {
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([Constraint::Percentage(50), Constraint::Percentage(50)])
        .split(area);

    // Time Chart
    let time_data: Vec<(&str, u64)> = app
        .benchmark_results
        .iter()
        .map(|r| (r.name.as_str(), (r.time * 1000.0) as u64))
        .collect();

    let time_chart = BarChart::default()
        .block(
            Block::default()
                .title("Execution Time (ms)")
                .borders(Borders::ALL),
        )
        .data(&time_data)
        .bar_width(10)
        .bar_gap(5)
        .value_style(Style::default().fg(Color::Black).bg(Color::White))
        .style(Style::default().fg(Color::Cyan));

    frame.render_widget(time_chart, chunks[0]);

    // Fragmentation Chart
    let frag_data: Vec<(&str, u64)> = app
        .benchmark_results
        .iter()
        .map(|r| (r.name.as_str(), r.total_blocks))
        .collect();

    let frag_chart = BarChart::default()
        .block(
            Block::default()
                .title("Total Block Count (Fragmentation)")
                .borders(Borders::ALL),
        )
        .data(&frag_data)
        .bar_width(10)
        .bar_gap(5)
        .value_style(Style::default().fg(Color::Black).bg(Color::White))
        .style(Style::default().fg(Color::Magenta));

    frame.render_widget(frag_chart, chunks[1]);
}
