use std::fmt::Display;

use eframe::{
    egui::{self, Ui},
    epaint::Color32,
};
use egui_extras::{Size, Table, TableBuilder};
use mysql::{prelude::*, PooledConn};

use super::{IntoTable, Selectable};

#[derive(Debug, Default)]
pub struct Vendors {
    vendor_id: String,
    surname: String,
    rating: usize,
    city: String,
}

impl Vendors {
    pub fn new(vendor_id: String, surname: String, rating: usize, city: String) -> Self {
        Self {
            vendor_id,
            surname,
            rating,
            city,
        }
    }

    pub fn all_ids(conn: &mut mysql::PooledConn) -> Vec<String> {
        conn.query("SELECT vendor_id from Vendors").unwrap()
    }

    pub fn is_presented(&self, conn: &mut mysql::PooledConn) -> bool {
        conn.query_first::<String, _>(format!(
            "SELECT vendor_id from Vendors where vendor_id = '{}'",
            self.vendor_id
        ))
        .unwrap()
        .is_some()
    }
}

impl Selectable for Vendors {
    type Item = Self;

    fn select_all(conn: &mut mysql::PooledConn) -> Vec<Self::Item> {
        conn.query_map(
            "SELECT * FROM Vendors",
            |(vendor_id, surname, rating, city)| Self::new(vendor_id, surname, rating, city),
        )
        .unwrap_or_default()
    }
}

impl IntoTable for Vendors {
    fn build(ui: &mut Ui) -> Table {
        TableBuilder::new(ui)
            .cell_layout(egui::Layout::centered_and_justified(
                egui::Direction::LeftToRight,
            ))
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

    fn display_table(ui: &mut Ui, conn: &mut PooledConn) {
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

    fn insert_row(&mut self, ui: &mut Ui, conn: &mut PooledConn) {
        let mut vend_builder = VendorsBuilder::new();
        ui.horizontal(|ui| {
            ui.label("Vendor Id: ");
            ui.text_edit_singleline(&mut self.vendor_id);
        });
        ui.horizontal(|ui| {
            ui.label("Surname: ");
            ui.text_edit_singleline(&mut self.surname);
        });
        ui.horizontal(|ui| {
            ui.label("City: ");
            ui.text_edit_singleline(&mut self.city);
        });
        ui.horizontal(|ui| {
            ui.label("Rating: ");
            ui.add(egui::Slider::new(&mut self.rating, 0..=100));
        });

        let vend = vend_builder
            .vendor_id(self.vendor_id.clone())
            .surname(self.surname.clone())
            .city(self.city.clone())
            .rating(self.rating)
            .build();

        let (can_insert, value) = match vend {
            Ok(part) => {
                if part.is_presented(conn) {
                    ui.colored_label(Color32::RED, "Вендор с таким id уже существует.");
                    (false, part)
                } else {
                    (true, part)
                }
            }
            Err(err) => {
                ui.colored_label(Color32::RED, err.to_string());
                (false, Vendors::default())
            }
        };

        if ui
            .add_enabled(can_insert, egui::Button::new("Вставить"))
            .clicked()
        {
            conn.query::<String, _>(format!(
                "INSERT INTO Vendors(vendor_id, surname, rating, city) VALUES ('{}', '{}', {}, '{}');",
                value.vendor_id,
                value.surname,
                value.rating,
                value.city
            )).unwrap();
            *self = Self::default();
        }
    }
}

enum VendorsError {
    VendorId,
    Surname,
    Rating,
    City,
}

impl Display for VendorsError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            VendorsError::VendorId => f.write_str("Не указан vendor_id"),
            VendorsError::Surname => f.write_str("Не указан surname"),
            VendorsError::Rating => f.write_str("Не указан rating"),
            VendorsError::City => f.write_str("Не указан city"),
        }
    }
}

#[derive(Default)]
pub struct VendorsBuilder {
    vendor_id: Option<String>,
    surname: Option<String>,
    rating: Option<usize>,
    city: Option<String>,
}

impl VendorsBuilder {
    pub fn new() -> Self {
        Self::default()
    }

    fn vendor_id(&mut self, vendor_id: impl Into<String>) -> &mut Self {
        self.vendor_id = Some(vendor_id.into());
        self
    }

    fn surname(&mut self, surname: impl Into<String>) -> &mut Self {
        self.surname = Some(surname.into());
        self
    }

    fn city(&mut self, city: impl Into<String>) -> &mut Self {
        self.city = Some(city.into());
        self
    }

    fn rating(&mut self, rating: usize) -> &mut Self {
        self.rating = Some(rating);
        self
    }

    fn build(&self) -> Result<Vendors, VendorsError> {
        let Some(vendor_id) = self.vendor_id.as_ref() else {
            return Err(VendorsError::VendorId);
        };
        if vendor_id.is_empty() {
            return Err(VendorsError::VendorId);
        }

        let Some(surname) = self.surname.as_ref() else {
            return Err(VendorsError::Surname);
        };
        if surname.is_empty() {
            return Err(VendorsError::Surname);
        }

        let Some(city) = self.city.as_ref() else {
            return Err(VendorsError::City);
        };
        if city.is_empty() {
            return Err(VendorsError::City);
        }

        let Some(rating) = self.rating else {
            return Err(VendorsError::Rating);
        };

        Ok(Vendors::new(
            vendor_id.to_string(),
            surname.to_string(),
            rating,
            city.to_string(),
        ))
    }
}
