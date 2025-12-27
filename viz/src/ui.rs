use crate::app::App;
use ratatui::{
    prelude::*,
    widgets::{BarChart, Block, Borders, Cell, Paragraph, Row, Table},
};

// Tokyo Night Colors
const TOKYO_BG: Color = Color::Rgb(26, 27, 38);
const TOKYO_FG: Color = Color::Rgb(192, 202, 245);
const TOKYO_RED: Color = Color::Rgb(247, 118, 142);
const TOKYO_GREEN: Color = Color::Rgb(158, 206, 106);
const TOKYO_YELLOW: Color = Color::Rgb(224, 175, 104);
const TOKYO_BLUE: Color = Color::Rgb(122, 162, 247);
const TOKYO_MAGENTA: Color = Color::Rgb(187, 154, 247);
const TOKYO_CYAN: Color = Color::Rgb(125, 207, 255);
const TOKYO_WHITE: Color = Color::Rgb(169, 177, 214);

pub fn ui(frame: &mut Frame, app: &App) {
    let area = frame.area();

    // Main Layout: Header, Content (Split), Footer
    let main_chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([
            Constraint::Length(3), // Header
            Constraint::Min(1),    // Content
            Constraint::Length(3), // Footer
        ])
        .split(area);

    // Header
    render_header(frame, app, main_chunks[0]);

    // Content Split: Visualizer (Top) and Stats (Bottom)
    let content_chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([
            Constraint::Percentage(60), // Visualizer
            Constraint::Percentage(40), // Stats
        ])
        .split(main_chunks[1]);

    render_visualizer(frame, app, content_chunks[0]);
    render_statistics(frame, app, content_chunks[1]);

    // Footer
    render_footer(frame, app, main_chunks[2]);
}

fn render_header(frame: &mut Frame, app: &App, area: Rect) {
    if app.steps.is_empty() {
        return;
    }
    let step = app.current_step();

    let header_text = format!(
        " Algorithm: {} | Step: {} | Op: {} | Highlight: {} ",
        step.algo, step.step, step.op, step.highlight
    );

    let header = Paragraph::new(header_text)
        .block(
            Block::default()
                .borders(Borders::ALL)
                .border_style(Style::default().fg(TOKYO_BLUE))
                .title(" MemFlex Viz ")
                .title_style(
                    Style::default()
                        .fg(TOKYO_MAGENTA)
                        .add_modifier(Modifier::BOLD),
                ),
        )
        .style(Style::default().fg(TOKYO_FG).bg(TOKYO_BG))
        .alignment(Alignment::Center);
    frame.render_widget(header, area);
}

fn render_footer(frame: &mut Frame, _app: &App, area: Rect) {
    let footer_text = " Controls: [n/Right] Next | [p/Left] Prev | [q] Quit ";
    let footer = Paragraph::new(footer_text)
        .block(
            Block::default()
                .borders(Borders::ALL)
                .border_style(Style::default().fg(TOKYO_BLUE)),
        )
        .style(Style::default().fg(TOKYO_FG).bg(TOKYO_BG))
        .alignment(Alignment::Center);
    frame.render_widget(footer, area);
}

fn render_visualizer(frame: &mut Frame, app: &App, area: Rect) {
    if app.steps.is_empty() {
        return;
    }
    let step = app.current_step();

    let block = Block::default()
        .borders(Borders::ALL)
        .border_style(Style::default().fg(TOKYO_CYAN))
        .title(" Memory Map ")
        .style(Style::default().bg(TOKYO_BG));

    frame.render_widget(block.clone(), area);

    let inner_area = block.inner(area);
    let width = inner_area.width as usize;

    let total_size: usize = step.blocks.iter().map(|b| b.size).sum();

    if total_size > 0 {
        let mut x_offset = inner_area.x;
        let mut y_offset = inner_area.y;
        let row_height = 3; // Height of block + border

        // Calculate scale factor to fit roughly in the view if possible,
        // but wrapping is better for detail.
        // Let's try to make 1 byte = N pixels? No, that's too big.
        // Let's stick to proportional width relative to screen width, but allow wrapping.
        // Actually, if we wrap, "proportional to screen width" doesn't make sense for the whole heap.
        // It should be proportional to the total size, spread across multiple lines.

        // Let's define a "virtual width" that is larger than the screen width if needed?
        // Or just map the total size to the available area (width * height)?

        // Let's try: Map total_size to (width * available_rows).
        // let available_rows = (inner_area.height / row_height as u16) as usize;
        // let total_capacity = width * available_rows;

        // If total_size is small, don't stretch it too much.
        // If total_size is huge, we might lose detail.

        // Let's just iterate and wrap.

        for block in &step.blocks {
            // Minimum width 2 for borders
            let mut block_width =
                (block.size as f64 / total_size as f64 * (width as f64 * 2.0)).round() as u16;
            // Multiplied by 2.0 to give it some more space, effectively zooming in 2x and wrapping

            block_width = block_width.max(4); // Min width to show label

            // Check if we need to wrap
            if x_offset + block_width > inner_area.x + inner_area.width {
                // Move to next line
                x_offset = inner_area.x;
                y_offset += row_height as u16;
            }

            // Check if we are out of vertical space
            if y_offset + row_height as u16 > inner_area.y + inner_area.height {
                break;
            }

            // Cap width at line end if it's a single huge block (or just let it be cut off?)
            // Better to cap it.
            let max_w = (inner_area.x + inner_area.width).saturating_sub(x_offset);
            let render_width = block_width.min(max_w);

            let rect = Rect::new(x_offset, y_offset, render_width, row_height as u16);

            let color = if block.is_free {
                TOKYO_GREEN
            } else {
                TOKYO_RED
            };

            let is_highlighted = block.addr == step.highlight;
            let style = if is_highlighted {
                Style::default()
                    .bg(color)
                    .fg(TOKYO_BG)
                    .add_modifier(Modifier::BOLD)
            } else {
                Style::default().bg(color).fg(TOKYO_BG)
            };

            let block_widget = Block::default()
                .borders(Borders::ALL)
                .border_style(Style::default().fg(if is_highlighted {
                    TOKYO_WHITE
                } else {
                    TOKYO_BG
                }))
                .style(style);

            frame.render_widget(block_widget, rect);

            if render_width > 4 {
                let label = format!("{}", block.size);
                let label_widget = Paragraph::new(label)
                    .style(Style::default().fg(TOKYO_BG).add_modifier(Modifier::BOLD))
                    .alignment(Alignment::Center);

                let label_rect = Rect::new(x_offset + 1, y_offset + 1, render_width - 2, 1);
                frame.render_widget(label_widget, label_rect);
            }

            x_offset += render_width;
        }
    }
}

fn render_statistics(frame: &mut Frame, app: &App, area: Rect) {
    let chunks = Layout::default()
        .direction(Direction::Horizontal)
        .constraints([Constraint::Percentage(50), Constraint::Percentage(50)])
        .split(area);

    // Left: Current Step Stats (Dynamic)
    render_current_stats(frame, app, chunks[0]);

    // Right: Global Benchmarks
    render_global_stats(frame, app, chunks[1]);
}

fn render_current_stats(frame: &mut Frame, app: &App, area: Rect) {
    if app.steps.is_empty() {
        return;
    }
    let step = app.current_step();

    // Calculate stats for current step
    let mut small_blocks = 0;
    let mut medium_blocks = 0;
    let mut large_blocks = 0;
    let mut used_bytes = 0;
    let mut free_bytes = 0;

    for block in &step.blocks {
        if block.size < 32 {
            small_blocks += 1;
        } else if block.size < 128 {
            medium_blocks += 1;
        } else {
            large_blocks += 1;
        }

        if block.is_free {
            free_bytes += block.size;
        } else {
            used_bytes += block.size;
        }
    }

    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([Constraint::Percentage(50), Constraint::Percentage(50)])
        .split(area);

    // 1. Block Size Distribution Histogram
    let data = vec![
        ("Small (<32)", small_blocks),
        ("Med (32-128)", medium_blocks),
        ("Large (>128)", large_blocks),
    ];

    let barchart = BarChart::default()
        .block(
            Block::default()
                .title(" Block Size Dist ")
                .borders(Borders::ALL)
                .border_style(Style::default().fg(TOKYO_YELLOW)),
        )
        .data(&data)
        .bar_width(12)
        .bar_gap(2)
        .style(Style::default().fg(TOKYO_YELLOW).bg(TOKYO_BG))
        .value_style(Style::default().fg(TOKYO_BG).bg(TOKYO_YELLOW));

    frame.render_widget(barchart, chunks[0]);

    // 2. Usage Stats (Used vs Free)
    let usage_data = vec![("Used", used_bytes as u64), ("Free", free_bytes as u64)];

    let usage_chart = BarChart::default()
        .block(
            Block::default()
                .title(" Memory Usage ")
                .borders(Borders::ALL)
                .border_style(Style::default().fg(TOKYO_RED)),
        )
        .data(&usage_data)
        .bar_width(10)
        .bar_gap(4)
        .style(Style::default().fg(TOKYO_RED).bg(TOKYO_BG))
        .value_style(Style::default().fg(TOKYO_BG).bg(TOKYO_RED));

    frame.render_widget(usage_chart, chunks[1]);
}

fn render_global_stats(frame: &mut Frame, app: &App, area: Rect) {
    let chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([
            Constraint::Length(6), // Table
            Constraint::Min(1),    // Charts
        ])
        .split(area);

    render_benchmark_table(frame, app, chunks[0]);

    let chart_chunks = Layout::default()
        .direction(Direction::Vertical)
        .constraints([Constraint::Percentage(50), Constraint::Percentage(50)])
        .split(chunks[1]);

    // Time Chart
    let time_data: Vec<(&str, u64)> = app
        .benchmark_results
        .iter()
        .map(|r| (r.name.as_str(), (r.time * 1000.0) as u64))
        .collect();

    let time_chart = BarChart::default()
        .block(
            Block::default()
                .title(" Step 3 Time (ms) ")
                .borders(Borders::ALL)
                .border_style(Style::default().fg(TOKYO_GREEN)),
        )
        .data(&time_data)
        .bar_width(10)
        .bar_gap(2)
        .value_style(Style::default().fg(TOKYO_BG).bg(TOKYO_GREEN))
        .style(Style::default().fg(TOKYO_GREEN).bg(TOKYO_BG));

    frame.render_widget(time_chart, chart_chunks[0]);

    // Fragmentation Chart (Total Blocks)
    let frag_data: Vec<(&str, u64)> = app
        .benchmark_results
        .iter()
        .map(|r| (r.name.as_str(), r.total_blocks))
        .collect();

    let frag_chart = BarChart::default()
        .block(
            Block::default()
                .title(" Total Blocks (Frag) ")
                .borders(Borders::ALL)
                .border_style(Style::default().fg(TOKYO_MAGENTA)),
        )
        .data(&frag_data)
        .bar_width(10)
        .bar_gap(2)
        .value_style(Style::default().fg(TOKYO_BG).bg(TOKYO_MAGENTA))
        .style(Style::default().fg(TOKYO_MAGENTA).bg(TOKYO_BG));

    frame.render_widget(frag_chart, chart_chunks[1]);
}

fn render_benchmark_table(frame: &mut Frame, app: &App, area: Rect) {
    let header_cells = ["Algo", "Step 3 Time (s)", "Blocks"]
        .iter()
        .map(|h| Cell::from(*h).style(Style::default().fg(TOKYO_BG).bg(TOKYO_BLUE)));
    let header = Row::new(header_cells)
        .style(Style::default().bg(TOKYO_BLUE))
        .height(1);

    let rows = app.benchmark_results.iter().map(|item| {
        let cells = vec![
            Cell::from(item.name.clone()),
            Cell::from(format!("{:.6}", item.time)),
            Cell::from(item.total_blocks.to_string()),
        ];
        Row::new(cells)
            .height(1)
            .style(Style::default().fg(TOKYO_FG))
    });

    let t = Table::new(
        rows,
        [
            Constraint::Percentage(30),
            Constraint::Percentage(40),
            Constraint::Percentage(30),
        ],
    )
    .header(header)
    .block(
        Block::default()
            .borders(Borders::ALL)
            .title(" Detailed Stats "),
    );

    frame.render_widget(t, area);
}
