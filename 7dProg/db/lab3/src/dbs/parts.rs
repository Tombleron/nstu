use eframe::egui::{self, Ui};
use egui_extras::{Size, Table, TableBuilder};
use mysql::prelude::*;
use mysql::*;

use super::{IntoTable, Selectable};

#[derive(Debug)]
pub struct Parts {
    part_id: String,
    part_name: String,
    color: String,
    weight: usize,
    city: String,
}

impl Parts {
    pub fn new(
        part_id: String,
        part_name: String,
        color: String,
        weight: usize,
        city: String,
    ) -> Self {
        Self {
            part_id,
            part_name,
            color,
            weight,
            city,
        }
    }
}

impl Selectable for Parts {
    type Item = Self;

    fn select_all(conn: &mut mysql::PooledConn) -> Vec<Self::Item> {
        conn.query_map(
            "SELECT * FROM Parts",
            |(part_id, part_name, color, weight, city)| {
                Self::new(part_id, part_name, color, weight, city)
            },
        )
        .unwrap_or_default()
    }
}

impl IntoTable for Parts {
    fn build(ui: &mut Ui) -> Table {
        TableBuilder::new(ui)
            .cell_layout(egui::Layout::left_to_right(egui::Align::Center))
            .column(Size::relative(0.225))
            .column(Size::relative(0.225))
            .column(Size::relative(0.225))
            .column(Size::relative(0.1))
            .column(Size::relative(0.225))
            .resizable(true)
            .header(20.0, |mut header| {
                header.col(|ui| {
                    ui.heading("part_id");
                });
                header.col(|ui| {
                    ui.heading("part_name");
                });
                header.col(|ui| {
                    ui.heading("color");
                });
                header.col(|ui| {
                    ui.heading("weight");
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
                        ui.label(elem.part_id.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.part_name.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.color.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.weight.to_string());
                    });
                    row.col(|ui| {
                        ui.label(elem.city.to_string());
                    });
                });
            }
        });
    }
}
