use eframe::egui::{self, Ui};
use egui_extras::{Size, Table, TableBuilder};
use mysql::prelude::*;
use mysql::*;

use super::{IntoTable, Selectable};

#[derive(Debug)]
pub struct Products {
    product_id: String,
    product_name: String,
    city: String,
}

impl Products {
    pub fn new(product_id: String, product_name: String, city: String) -> Self {
        Self {
            product_id,
            product_name,
            city,
        }
    }
}

impl Selectable for Products {
    type Item = Self;

    fn select_all(conn: &mut mysql::PooledConn) -> Vec<Self::Item> {
        conn.query_map(
            "SELECT * FROM Products",
            |(product_id, product_name, city)| Self::new(product_id, product_name, city),
        )
        .unwrap_or_default()
    }
}

impl IntoTable for Products {
    fn build(ui: &mut Ui) -> Table {
        TableBuilder::new(ui)
            .cell_layout(egui::Layout::left_to_right(egui::Align::Center))
            .column(Size::relative(0.25))
            .column(Size::relative(0.55))
            .column(Size::relative(0.25))
            .resizable(true)
            .header(20.0, |mut header| {
                header.col(|ui| {
                    ui.heading("product_id");
                });
                header.col(|ui| {
                    ui.heading("product_name");
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
                        ui.label(elem.product_id.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.product_name.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.city.to_string());
                    });
                });
            }
        });
    }
}
