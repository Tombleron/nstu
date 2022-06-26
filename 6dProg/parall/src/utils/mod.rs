use bincode;
use md5::{self, Digest};
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
pub struct WorkDone(pub usize);

#[derive(Serialize, Deserialize, Debug)]
pub enum ServerPacket {
    Thanks,
    Work(Work),
    Empty,
}

#[derive(Serialize, Deserialize, Debug)]
pub enum WorkerPacket {
    Available,
    Ready(Work),
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct Block {
    block_id: usize,
    nonce: usize,
    difficulty: usize,
    message: String,
}

impl PartialEq for Block {
    fn eq(&self, other: &Self) -> bool {
        self.block_id == other.block_id && self.difficulty == other.difficulty && self.message == other.message
    }
}

impl Block {
    pub fn new() -> Self {
        Self {
            block_id: 0,
            nonce: 0,
            difficulty: 3,
            message: "First block".to_string(),
        }
    }

    pub fn new_with_id(block_id: usize) -> Self {
        Self {
            block_id,
            nonce: 0,
            difficulty: 2,
            message: format!("Block no. {block_id}"),
        }
    }

    fn into_bin(&self) -> Vec<u8> {
        bincode::serialize(self).unwrap()
    }

    pub fn hash(&self) -> Digest {
        md5::compute(self.into_bin())
    }

    pub fn check_hash(&self) -> bool {
        self.hash().iter().take(self.difficulty).all(|&x| x == 0)
    }
    
    pub fn set_nonce(&mut self, nonce: usize) {
        self.nonce = nonce;
    }

    pub fn inc_nonce(&mut self) {
        self.nonce += 1;
    }
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct Work {
    id: usize,
    block: Block,
}

impl Work {
    pub fn new() -> Self {
        Self {
            id: 0,
            block: Block::new(),
        }
    }

    pub fn id(&self) -> usize {
        self.id
    }

    pub fn block(&self) -> &Block {
        &self.block
    }

    pub fn block_mut(&mut self) -> &mut Block {
        &mut self.block
    }

    pub fn next(&self) -> Self {
        Self {
            id: self.id() + 1,
            block: Block::new_with_id(self.id() + 1),
        }
    }

    pub fn validate(&self, current: &Work) -> bool {
        if self.block == current.block && self.id() == current.id() {
            self.block.check_hash()
        } else {
            false
        }
    }
}
