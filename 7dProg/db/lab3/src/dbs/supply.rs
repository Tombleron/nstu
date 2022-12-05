use eframe::egui::{self, Ui};
use egui_extras::{Size, Table, TableBuilder};
use mysql::prelude::*;
use mysql::*;

use super::{IntoTable, Selectable};

#[derive(Debug)]
pub struct Supply {
    id: usize,
    vendor_id: String,
    part_id: String,
    product_id: String,
    count: usize,
}

impl Supply {
    fn new(
        id: usize,
        vendor_id: String,
        part_id: String,
        product_id: String,
        count: usize,
    ) -> Self {
        Self {
            id,
            vendor_id,
            part_id,
            product_id,
            count,
        }
    }
}

impl Selectable for Supply {
    type Item = Self;

    fn select_all(conn: &mut mysql::PooledConn) -> Vec<Self::Item> {
        conn.query_map(
            "SELECT * FROM Supply",
            |(id, vendor_id, part_id, product_id, count)| {
                Self::new(id, vendor_id, part_id, product_id, count)
            },
        )
        .unwrap_or_default()
    }
}

impl IntoTable for Supply {
    fn build(ui: &mut Ui) -> Table {
        TableBuilder::new(ui)
            .cell_layout(egui::Layout::left_to_right(egui::Align::Center))
            .column(Size::relative(0.225))
            .column(Size::relative(0.225))
            .column(Size::relative(0.225))
            .column(Size::relative(0.225))
            .column(Size::relative(0.1))
            .resizable(true)
            .header(20.0, |mut header| {
                header.col(|ui| {
                    ui.heading("id");
                });
                header.col(|ui| {
                    ui.heading("vendor_id");
                });
                header.col(|ui| {
                    ui.heading("part_id");
                });
                header.col(|ui| {
                    ui.heading("product_id");
                });
                header.col(|ui| {
                    ui.heading("count");
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
                        ui.label(elem.id.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.vendor_id.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.part_id.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.product_id.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.count.to_string());
                    });
                });
            }
        });
    }
}
