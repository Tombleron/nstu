use eframe::egui::Ui;
use egui_extras::Table;
use mysql::PooledConn;

pub mod parts;
pub mod products;
pub mod supply;
pub mod vendors;

pub trait Selectable {
    type Item;
    fn select_all(conn: &mut PooledConn) -> Vec<Self::Item>;
}

pub enum TableError {
    Dummy
}

pub trait IntoTable {
    fn build(ui: &mut Ui) -> Table;
    fn display_table(ui: &mut Ui, conn: &mut PooledConn);
    fn insert_row(&mut self, ui: &mut Ui, conn: &mut PooledConn) -> Result<(), TableError>;
}
