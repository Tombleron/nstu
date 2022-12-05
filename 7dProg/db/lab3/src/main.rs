mod dbs;
use dbs::parts::Parts;
use dbs::products::Products;
use dbs::supply::Supply;
use dbs::vendors::Vendors;
use dbs::*;
use eframe::egui;

use mysql::*;

use std::fmt::Display;

struct MyApp {
    state: States,
    conn: PooledConn,
}

fn main() -> std::result::Result<(), Box<dyn std::error::Error>> {
    // Log to stdout (if you run with `RUST_LOG=debug`).
    tracing_subscriber::fmt::init();

    let url = "mysql://tombleron:test@localhost:6603/IB";
    let pool = Pool::new(url)?;
    let conn = pool.get_conn()?;

    let options = eframe::NativeOptions {
        initial_window_size: Some(egui::vec2(320.0, 240.0)),
        ..Default::default()
    };
    eframe::run_native(
        "lab3",
        options,
        Box::new(|_cc| Box::new(MyApp::new(States::View(Tables::Supply), conn))),
    );

    Ok(())
}

#[derive(Debug)]
enum Tables {
    Supply,
    Vendors,
    Parts,
    Products,
}

impl Display for Tables {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Tables::Supply => f.write_str("Supply"),
            Tables::Vendors => f.write_str("Vendors"),
            Tables::Parts => f.write_str("Parts"),
            Tables::Products => f.write_str("Products"),
        }
    }
}

#[derive(Debug)]
enum States {
    View(Tables),
    Search,
}

impl States {
    pub fn update(&self, ctx: &egui::Context, conn: &mut PooledConn) {
        // Generic code for all states
        match self {
            States::View(table) => self.view(ctx, conn, table),
            States::Search => self.search(ctx, conn),
        }
    }
    fn view(&self, ctx: &egui::Context, conn: &mut PooledConn, table: &Tables) {
        egui::CentralPanel::default().show(ctx, |ui| match table {
            Tables::Supply => {
                Supply::create_table(ui, conn);
            }
            Tables::Parts => {
                Parts::create_table(ui, conn);
            }
            Tables::Vendors => {
                Vendors::create_table(ui, conn);
            }
            Tables::Products => {
                Products::create_table(ui, conn);
            }
        });
    }

    fn search(&self, ctx: &egui::Context, _conn: &mut PooledConn) {
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("lab3");
            ui.label(format!("State: {:?}", self));
        });
    }
}

impl MyApp {
    fn new(state: States, conn: PooledConn) -> Self {
        Self { state, conn }
    }
    fn change_state(&mut self, state: States) {
        self.state = state
    }
}

impl eframe::App for MyApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        egui::TopBottomPanel::top("top_panel").show(ctx, |ui| {
            egui::menu::bar(ui, |ui| {
                ui.menu_button("File", |ui| {
                    if ui.button("Quit").clicked() {
                        _frame.close();
                    }
                });

                ui.menu_button("Tables", |ui| {
                    if ui.button("Supply").clicked() {
                        self.change_state(States::View(Tables::Supply))
                    }
                    if ui.button("Vendors").clicked() {
                        self.change_state(States::View(Tables::Vendors))
                    }
                    if ui.button("Parts").clicked() {
                        self.change_state(States::View(Tables::Parts))
                    }
                    if ui.button("Products").clicked() {
                        self.change_state(States::View(Tables::Products))
                    }
                });
                if ui.button("Search").clicked() {
                    self.change_state(States::Search)
                }
            });
        });

        self.state.update(ctx, &mut self.conn);
    }
}
