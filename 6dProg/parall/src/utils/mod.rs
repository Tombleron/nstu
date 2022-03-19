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
        for &i in self.hash().iter().take(self.difficulty) {
            if i != 0 {
                return false;
            }
        }

        true
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
    work_id: usize,
    block: Block,
}

impl Work {
    pub fn id(&self) -> usize {
        self.work_id
    }

    pub fn block(&mut self) -> &mut Block {
        &mut self.block
    }

    pub fn new() -> Self {
        Self {
            work_id: 0,
            block: Block::new(),
        }
    }

    pub fn next(&self) -> Self {
        Self {
            work_id: self.work_id + 1,
            block: Block::new_with_id(self.work_id + 1),
        }
    }

    pub fn validate(&self, current: &Work) -> bool {
        if self.block == current.block && self.work_id == current.work_id {
            self.block.check_hash()
        } else {
            false
        }
    }
}
