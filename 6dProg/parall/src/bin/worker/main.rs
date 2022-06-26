use parall::utils::*;
use rand::random;
use std::{env, thread};
use tracing::{info, Level};
use tracing_subscriber::FmtSubscriber;
use zmq::Socket;

struct Worker {
    req_socket: Socket,
    sub_socket: Socket,
}

impl Worker {
    fn new() -> Self {
        let ctx = zmq::Context::new();

        let req_socket = ctx.socket(zmq::REQ).expect("Unable to create req socket.");
        let sub_socket = ctx.socket(zmq::SUB).expect("Unable to create sub socket.");

        Self {
            req_socket,
            sub_socket,
        }
    }

    fn run(&mut self) {
        self.req_socket
            .connect("tcp://127.0.0.1:5557")
            .expect("Unable to connect to validator socket.");

        self.sub_socket
            .connect("tcp://127.0.0.1:5555")
            .expect("Unable to connect to pub socket.");

        let mut msg = zmq::Message::new();

        loop {
            self.req_socket
                .send(&serde_json::to_string(&WorkerPacket::Available).unwrap(), 0)
                .expect("Unable to send message.");

            self.req_socket
                .recv(&mut msg, 0)
                .expect("Unable to retrive message.");

            let result: ServerPacket = serde_json::from_str(msg.as_str().unwrap()).unwrap();
            info!("Got message from the server: {:?}", result);

            match result {
                ServerPacket::Thanks => {}
                ServerPacket::Empty => {}
                ServerPacket::Work(mut work) => {
                    info!("Got work: {:?}", work);
                    self.process_packet(&mut work);
                    self.req_socket
                        .send(
                            &serde_json::to_string(&WorkerPacket::Ready(work)).unwrap(),
                            0,
                        )
                        .expect("Unable to send message.");
                    self.req_socket
                        .recv(&mut msg, 0)
                        .expect("Unable to retrive message.");
                }
            }
        }
    }

    fn process_packet(&self, work: &mut Work) {
        work.block_mut().set_nonce(random());

        while !work.block_mut().check_hash() {
            work.block_mut().inc_nonce();
        }

        info!("Nonce found!");
    }
}

fn main() {
    if env::args().len() < 2 {
        println!("Usage: {} <workers_count>", env::args().last().unwrap());
    } else {
        let subscriber = FmtSubscriber::builder()
            .with_max_level(Level::TRACE)
            .with_thread_ids(true)
            .finish();

        tracing::subscriber::set_global_default(subscriber)
            .expect("setting default subscriber failed");
        let mut handlers = Vec::new();
        let count = env::args().last().unwrap().parse::<usize>().unwrap();
        for i in 0..count {
            info!("Spawning worker with id: {i}");
            handlers.push(thread::spawn(move || Worker::new().run()));
        }

        for handler in handlers {
            handler.join().unwrap();
        }
    }
}
