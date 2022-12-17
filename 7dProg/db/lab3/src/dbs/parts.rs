use std::fmt::Display;

use eframe::{
    egui::{self, Ui},
    epaint::Color32,
};
use egui_extras::{Size, Table, TableBuilder};
use mysql::{prelude::*, PooledConn};
// use mysql::*;

use super::{IntoTable, Selectable};

#[derive(Debug, Default)]
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

    pub fn all_ids(conn: &mut mysql::PooledConn) -> Vec<String> {
        conn.query("SELECT part_id from Parts").unwrap()
    }

    pub fn is_presented(&self, conn: &mut mysql::PooledConn) -> bool {
        conn.query_first::<String, _>(format!(
            "SELECT part_id from Parts where part_id = '{}'",
            self.part_id
        ))
        .unwrap()
        .is_some()
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
            .cell_layout(egui::Layout::centered_and_justified(
                egui::Direction::LeftToRight,
            ))
            .column(Size::relative(0.225))
            .column(Size::relative(0.225))
            .column(Size::relative(0.225))
            .column(Size::relative(0.1))
            .column(Size::remainder())
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

    fn display_table(ui: &mut Ui, conn: &mut PooledConn) {
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

    fn insert_row(&mut self, ui: &mut Ui, conn: &mut PooledConn) {
        let mut part_builder = PartsBuilder::new();
        ui.horizontal(|ui| {
            ui.label("Part Id: ");
            ui.text_edit_singleline(&mut self.part_id);
        });
        ui.horizontal(|ui| {
            ui.label("Part Name: ");
            ui.text_edit_singleline(&mut self.part_name);
        });
        ui.horizontal(|ui| {
            ui.label("City: ");
            ui.text_edit_singleline(&mut self.city);
        });
        ui.horizontal(|ui| {
            ui.label("Color: ");
            ui.text_edit_singleline(&mut self.color);
        });
        ui.horizontal(|ui| {
            ui.label("Weight: ");
            ui.add(egui::Slider::new(&mut self.weight, 1..=1000));
        });

        let part = part_builder
            .part_id(self.part_id.clone())
            .part_name(self.part_name.clone())
            .color(self.color.clone())
            .city(self.city.clone())
            .weight(self.weight)
            .build();

        let (can_insert, value) = match part {
            Ok(part) => {
                if part.is_presented(conn) {
                    ui.colored_label(Color32::RED, "Деталь с таким id уже существует.");
                    (false, part)
                } else {
                    (true, part)
                }
            }
            Err(err) => {
                ui.colored_label(Color32::RED, err.to_string());
                (false, Parts::default())
            }
        };

        if ui
            .add_enabled(can_insert, egui::Button::new("Вставить"))
            .clicked()
        {
            conn.query::<String, _>(format!(
                "INSERT INTO Parts(part_id, part_name, color, weight, city) VALUES ('{}' ,'{}', '{}', {}, '{}');",
                value.part_id,
                value.part_name,
                value.color,
                value.weight,
                value.city
            )).unwrap();
            *self = Self::default();
        }
    }
}

enum PartsError {
    PartId,
    PartName,
    Color,
    Weight,
    City,
}

impl Display for PartsError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            PartsError::PartId => f.write_str("Не указан part_id"),
            PartsError::PartName => f.write_str("Не указан part_name"),
            PartsError::Color => f.write_str("Не указан color"),
            PartsError::Weight => f.write_str("Не указан weight"),
            PartsError::City => f.write_str("Не указан city"),
        }
    }
}

#[derive(Default)]
pub struct PartsBuilder {
    part_id: Option<String>,
    part_name: Option<String>,
    color: Option<String>,
    weight: Option<usize>,
    city: Option<String>,
}

impl PartsBuilder {
    pub fn new() -> Self {
        Self::default()
    }

    fn part_id(&mut self, part_id: impl Into<String>) -> &mut Self {
        self.part_id = Some(part_id.into());
        self
    }

    fn part_name(&mut self, part_name: impl Into<String>) -> &mut Self {
        self.part_name = Some(part_name.into());
        self
    }

    fn color(&mut self, color: impl Into<String>) -> &mut Self {
        self.color = Some(color.into());
        self
    }

    fn city(&mut self, city: impl Into<String>) -> &mut Self {
        self.city = Some(city.into());
        self
    }

    fn weight(&mut self, weight: usize) -> &mut Self {
        self.weight = Some(weight);
        self
    }

    fn build(&self) -> Result<Parts, PartsError> {
        let Some(part_id) = self.part_id.as_ref() else {
            return Err(PartsError::PartId);
        };
        if part_id.is_empty() {
            return Err(PartsError::PartId);
        }

        let Some(part_name) = self.part_name.as_ref() else {
            return Err(PartsError::PartName);
        };
        if part_name.is_empty() {
            return Err(PartsError::PartName);
        }

        let Some(city) = self.city.as_ref() else {
            return Err(PartsError::City);
        };
        if city.is_empty() {
            return Err(PartsError::City);
        }

        let Some(color) = self.color.as_ref() else {
            return Err(PartsError::Color);
        };
        if color.is_empty() {
            return Err(PartsError::Color);
        }

        let Some(weight) = self.weight else {
            return Err(PartsError::Weight);
        };
        if weight == 0 {
            return Err(PartsError::Weight);
        }

        Ok(Parts::new(
            part_id.to_string(),
            part_name.to_string(),
            color.to_string(),
            weight,
            city.to_string(),
        ))
    }
}
