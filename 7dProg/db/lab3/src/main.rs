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
    state: TableExplorer,
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
        Box::new(|_cc| {
            let a = TableExplorer {
                state: States::View(Tables::Supply(Supply::default())),
                dialog: false,
            };
            Box::new(MyApp::new(a, conn))
        }),
    );

    Ok(())
}

#[derive(Debug, Clone)]
enum Tables {
    Supply(Supply),
    Vendors(Vendors),
    Parts(Parts),
    Products(Products),
}

impl Display for Tables {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Tables::Supply(_) => f.write_str("Supply"),
            Tables::Vendors(_) => f.write_str("Vendors"),
            Tables::Parts(_) => f.write_str("Parts"),
            Tables::Products(_) => f.write_str("Products"),
        }
    }
}

struct TableExplorer {
    state: States,
    dialog: bool,
}

#[derive(Debug, Clone)]
enum States {
    View(Tables),
    Insert(Tables),
    Search,
}

impl TableExplorer {
    pub fn update(&mut self, ctx: &egui::Context, conn: &mut PooledConn) {
        // Generic code for all states
        match self.state {
            States::View(_) => self.view(ctx, conn),
            States::Insert(_) => self.insert(ctx, conn),
            States::Search => self.search(ctx, conn),
        }
    }
    fn view(&mut self, ctx: &egui::Context, conn: &mut PooledConn) {
        let table = match &self.state {
            States::View(table) => table,
            _ => unreachable!(),
        };

        egui::CentralPanel::default().show(ctx, |ui| match table {
            Tables::Supply(_) => {
                Supply::display_table(ui, conn);
            }
            Tables::Parts(_) => {
                Parts::display_table(ui, conn);
            }
            Tables::Vendors(_) => {
                Vendors::display_table(ui, conn);
            }
            Tables::Products(_) => {
                Products::display_table(ui, conn);
            }
        });
    }

    fn insert(&mut self, ctx: &egui::Context, conn: &mut PooledConn) {
        // FIXME: this is ugly
        let dialog = match &mut self.state {
            States::Insert(ref mut table) => {
                egui::CentralPanel::default()
                    .show(ctx, |ui| match table {
                        Tables::Supply(ref mut table) => table.insert_row(ui, conn),
                        Tables::Parts(ref mut table) => table.insert_row(ui, conn),
                        Tables::Vendors(ref mut table) => table.insert_row(ui, conn),
                        Tables::Products(ref mut table) => table.insert_row(ui, conn),
                    })
                    .inner
            }
            _ => unreachable!(),
        };

        self.dialog = match dialog {
            Ok(_) => true,
            Err(_) => false,
        } || self.dialog;

        if self.dialog {
            egui::Window::new("Вставка успешно выполнена.")
                .collapsible(false)
                .resizable(false)
                .show(ctx, |ui| {
                    ui.horizontal(|ui| {
                        if ui.button("Ok?").clicked() {
                            self.dialog = false;
                        };
                    });
                });
        }
    }

    fn search(&self, ctx: &egui::Context, _conn: &mut PooledConn) {
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("lab3");
            ui.label(format!("State: {:?}", self.state));
        });
    }
}

impl MyApp {
    fn new(state: TableExplorer, conn: PooledConn) -> Self {
        Self { state, conn }
    }
    fn change_state(&mut self, state: States) {
        self.state.state = state
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
                        self.change_state(States::View(Tables::Supply(Supply::default())))
                    }
                    if ui.button("Vendors").clicked() {
                        self.change_state(States::View(Tables::Vendors(Vendors::default())))
                    }
                    if ui.button("Parts").clicked() {
                        self.change_state(States::View(Tables::Parts(Parts::default())))
                    }
                    if ui.button("Products").clicked() {
                        self.change_state(States::View(Tables::Products(Products::default())))
                    }
                });

                ui.menu_button("Insert", |ui| {
                    if ui.button("Supply").clicked() {
                        self.change_state(States::Insert(Tables::Supply(Supply::default())))
                    }
                    if ui.button("Vendors").clicked() {
                        self.change_state(States::Insert(Tables::Vendors(Vendors::default())))
                    }
                    if ui.button("Parts").clicked() {
                        self.change_state(States::Insert(Tables::Parts(Parts::default())))
                    }
                    if ui.button("Products").clicked() {
                        self.change_state(States::Insert(Tables::Products(Products::default())))
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
