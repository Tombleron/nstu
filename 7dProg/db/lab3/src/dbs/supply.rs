use std::fmt::Display;

use eframe::{
    egui::{self, Ui},
    epaint::Color32,
};
use egui_extras::{Size, Table, TableBuilder};
use mysql::{prelude::*, PooledConn};

use super::{parts::Parts, products::Products, vendors::Vendors, IntoTable, Selectable, TableError};

#[derive(Debug, Default, Clone)]
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
            .cell_layout(egui::Layout::centered_and_justified(
                egui::Direction::LeftToRight,
            ))
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

    fn display_table(ui: &mut Ui, conn: &mut PooledConn) {
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

    fn insert_row(&mut self, ui: &mut Ui, conn: &mut PooledConn) -> Result<(), TableError> {
        ui.horizontal(|ui| {
            ui.label("Vendor Id: ");
            egui::ComboBox::from_id_source(120)
                .selected_text(self.vendor_id.to_string())
                .show_ui(ui, |ui| {
                    for id in Vendors::all_ids(conn) {
                        ui.selectable_value(&mut self.vendor_id, id.to_string(), id);
                    }
                });
        });
        ui.horizontal(|ui| {
            ui.label("Part Id: ");
            egui::ComboBox::from_id_source(901)
                .selected_text(self.part_id.to_string())
                .show_ui(ui, |ui| {
                    for id in Parts::all_ids(conn) {
                        ui.selectable_value(&mut self.part_id, id.to_string(), id);
                    }
                });
        });
        ui.horizontal(|ui| {
            ui.label("Product Id: ");
            egui::ComboBox::from_id_source(241)
                .selected_text(self.product_id.to_string())
                .show_ui(ui, |ui| {
                    for id in Products::all_ids(conn) {
                        ui.selectable_value(&mut self.product_id, id.to_string(), id);
                    }
                });
        });
        ui.horizontal(|ui| {
            ui.label("Count: ");
            ui.add(egui::Slider::new(&mut self.count, 1..=1000));
        });

        let supply = SupplyBuilder::new()
            .vendor_id(self.vendor_id.clone())
            .part_id(self.part_id.clone())
            .product_id(self.product_id.clone())
            .count(self.count)
            .build();

        let (can_insert, value) = match supply {
            Ok(part) => (true, part),
            Err(err) => {
                ui.colored_label(Color32::RED, err.to_string());
                (false, Supply::default())
            }
        };

        if ui
            .add_enabled(can_insert, egui::Button::new("Вставить"))
            .clicked()
        {
            conn.query::<String, _>(format!(
                "INSERT INTO Supply(vendor_id, part_id, product_id, count) VALUES ('{}' ,'{}', '{}', {});",
                value.vendor_id,
                value.part_id,
                value.product_id,
                value.count
            )).unwrap();
            *self = Self::default();

            return Ok(())
        }

        Err(TableError::Dummy)
    }
}

#[derive(Default)]
pub struct SupplyBuilder {
    vendor_id: Option<String>,
    part_id: Option<String>,
    product_id: Option<String>,
    count: Option<usize>,
}

enum SupplyError {
    VendorId,
    PartId,
    ProductId,
    Count,
}

impl Display for SupplyError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            SupplyError::VendorId => f.write_str("Не указан vendor_id"),
            SupplyError::PartId => f.write_str("Не указан part_id"),
            SupplyError::ProductId => f.write_str("Не указан product_id"),
            SupplyError::Count => f.write_str("Не указан count"),
        }
    }
}

impl SupplyBuilder {
    pub fn new() -> Self {
        Self::default()
    }

    fn part_id(&mut self, part_id: impl Into<String>) -> &mut Self {
        self.part_id = Some(part_id.into());
        self
    }

    fn vendor_id(&mut self, vendor_id: impl Into<String>) -> &mut Self {
        self.vendor_id = Some(vendor_id.into());
        self
    }

    fn product_id(&mut self, product_id: impl Into<String>) -> &mut Self {
        self.product_id = Some(product_id.into());
        self
    }

    fn count(&mut self, count: usize) -> &mut Self {
        self.count = Some(count);
        self
    }

    fn build(&self) -> Result<Supply, SupplyError> {
        let Some(part_id) = self.part_id.as_ref() else {
            return Err(SupplyError::PartId);
        };
        if part_id.is_empty() {
            return Err(SupplyError::PartId);
        }

        let Some(vendor_id) = self.vendor_id.as_ref() else {
            return Err(SupplyError::VendorId);
        };
        if vendor_id.is_empty() {
            return Err(SupplyError::VendorId);
        }

        let Some(product_id) = self.product_id.as_ref() else {
            return Err(SupplyError::ProductId);
        };
        if product_id.is_empty() {
            return Err(SupplyError::ProductId);
        }

        let Some(count) = self.count else {
            return Err(SupplyError::Count);
        };
        if count == 0 {
            return Err(SupplyError::Count);
        }

        Ok(Supply::new(
            0,
            vendor_id.to_string(),
            part_id.to_string(),
            product_id.to_string(),
            count,
        ))
    }
}
