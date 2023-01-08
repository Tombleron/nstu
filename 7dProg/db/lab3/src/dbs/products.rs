use std::fmt::Display;

use eframe::{
    egui::{self, Ui},
    epaint::Color32,
};
use egui_extras::{Size, Table, TableBuilder};
use mysql::{prelude::*, PooledConn};

use super::{IntoTable, Selectable, TableError};

#[derive(Debug, Default, Clone)]
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

    pub fn all_ids(conn: &mut mysql::PooledConn) -> Vec<String> {
        conn.query("SELECT product_id from Products").unwrap()
    }

    pub fn is_presented(&self, conn: &mut mysql::PooledConn) -> bool {
        conn.query_first::<String, _>(format!(
            "SELECT product_id from Products where product_id = '{}'",
            self.product_id
        ))
        .unwrap()
        .is_some()
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
            .cell_layout(egui::Layout::centered_and_justified(
                egui::Direction::LeftToRight,
            ))
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

    fn display_table(ui: &mut Ui, conn: &mut PooledConn) {
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

    fn insert_row(&mut self, ui: &mut Ui, conn: &mut PooledConn) -> Result<(), TableError> {
        let mut product_builder = ProductsBuilder::new();
        ui.horizontal(|ui| {
            ui.label("Product Id: ");
            ui.text_edit_singleline(&mut self.product_id);
        });
        ui.horizontal(|ui| {
            ui.label("Product Name: ");
            ui.text_edit_singleline(&mut self.product_name);
        });
        ui.horizontal(|ui| {
            ui.label("City: ");
            ui.text_edit_singleline(&mut self.city);
        });

        let product = product_builder
            .product_id(self.product_id.clone())
            .product_name(self.product_name.clone())
            .city(self.city.clone())
            .build();

        let (can_insert, value) = match product {
            Ok(product) => {
                if product.is_presented(conn) {
                    ui.colored_label(Color32::RED, "Продукт с таким id уже существует.");
                    (false, product)
                } else {
                    (true, product)
                }
            }
            Err(err) => {
                ui.colored_label(Color32::RED, err.to_string());
                (false, Products::default())
            }
        };

        if ui
            .add_enabled(can_insert, egui::Button::new("Вставить"))
            .clicked()
        {
            conn.query::<String, _>(format!(
                "INSERT INTO Products(product_id, product_name, city) VALUES ('{}' ,'{}', '{}');",
                value.product_id, value.product_name, value.city
            ))
            .unwrap();
            *self = Self::default();

            return Ok(());
        }

        Err(TableError::Dummy)
    }
}

enum ProductsError {
    ProductId,
    ProductName,
    City,
}

impl Display for ProductsError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            ProductsError::ProductId => f.write_str("Не указан product_id"),
            ProductsError::ProductName => f.write_str("Не указан product_name"),
            ProductsError::City => f.write_str("Не указан city"),
        }
    }
}

#[derive(Default)]
pub struct ProductsBuilder {
    product_id: Option<String>,
    product_name: Option<String>,
    city: Option<String>,
}

impl ProductsBuilder {
    pub fn new() -> Self {
        Self::default()
    }

    fn product_id(&mut self, product_id: impl Into<String>) -> &mut Self {
        self.product_id = Some(product_id.into());
        self
    }

    fn product_name(&mut self, product_name: impl Into<String>) -> &mut Self {
        self.product_name = Some(product_name.into());
        self
    }

    fn city(&mut self, city: impl Into<String>) -> &mut Self {
        self.city = Some(city.into());
        self
    }

    fn build(&self) -> Result<Products, ProductsError> {
        let Some(product_id) = self.product_id.as_ref() else {
            return Err(ProductsError::ProductId);
        };
        if product_id.is_empty() {
            return Err(ProductsError::ProductId);
        }

        let Some(product_name) = self.product_name.as_ref() else {
            return Err(ProductsError::ProductName);
        };
        if product_name.is_empty() {
            return Err(ProductsError::ProductName);
        }

        let Some(city) = self.city.as_ref() else {
            return Err(ProductsError::City);
        };
        if city.is_empty() {
            return Err(ProductsError::City);
        }

        Ok(Products::new(
            product_id.to_string(),
            product_name.to_string(),
            city.to_string(),
        ))
    }
}
