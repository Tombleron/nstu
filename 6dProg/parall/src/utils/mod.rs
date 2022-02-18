use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
pub enum Work {
    Thanks,
    Work((usize, usize)),
    Empty,
}

#[derive(Serialize, Deserialize)]
pub enum State {
    Availabe,
    Ready(usize),
}
