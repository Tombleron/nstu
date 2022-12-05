use eframe::egui::{self, Ui};
use egui_extras::{Size, Table, TableBuilder};
use mysql::prelude::*;
use mysql::*;

use super::{IntoTable, Selectable};

#[derive(Debug)]
pub struct Vendors {
    vendor_id: String,
    surname: String,
    rating: usize,
    city: String,
}

impl Vendors {
    pub fn new(vendor_id: String, surname: String, rating: usize, city: String) -> Self { Self { vendor_id, surname, rating, city } }
}

impl Selectable for Vendors {
    type Item = Self;

    fn select_all(conn: &mut mysql::PooledConn) -> Vec<Self::Item> {
        conn.query_map(
            "SELECT * FROM Vendors",
            |(vendor_id, surname, rating, city)| {
                Self::new(vendor_id, surname, rating, city)
            },
        )
        .unwrap_or_default()
    }
}

impl IntoTable for Vendors {
    fn build(ui: &mut Ui) -> Table {
        TableBuilder::new(ui)
            .cell_layout(egui::Layout::left_to_right(egui::Align::Center))
            .column(Size::relative(0.25))
            .column(Size::relative(0.25))
            .column(Size::relative(0.25))
            .column(Size::relative(0.25))
            .resizable(true)
            .header(20.0, |mut header| {
                header.col(|ui| {
                    ui.heading("vendor_id");
                });
                header.col(|ui| {
                    ui.heading("surname");
                });
                header.col(|ui| {
                    ui.heading("rating");
                });
                header.col(|ui| {
                    ui.heading("city");
                });
            })
    }

    fn create_table(ui: &mut Ui, conn: &mut PooledConn) {
        let table = Self::build(ui);
        let query = Self::select_all(conn);

        table.body(|mut body| {
            for elem in query.iter() {
                body.row(30.0, |mut row| {
                    row.col(|ui| {
                        ui.label(elem.vendor_id.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.surname.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.rating.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.city.to_string());
                    });
                });
            }
        });
    }
}
