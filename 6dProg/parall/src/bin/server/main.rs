use parall::utils::*;
use tracing::{info, Level};
use tracing_subscriber::FmtSubscriber;
use zmq::Socket;

struct Server {
    current_work: Work,
    rep_socket: Socket,
    pub_socket: Socket,
}

impl Server {
    fn new() -> Self {
        let ctx = zmq::Context::new();
        let rep_socket = ctx.socket(zmq::REP).expect("Unable to create rep socket.");
        let pub_socket = ctx.socket(zmq::PUB).expect("Unable to create pub socket.");
        Self {
            current_work: Work::new(),
            rep_socket,
            pub_socket,
        }
    }

    fn run(&mut self) {
        self.rep_socket
            .bind("tcp://0.0.0.0:5557")
            .expect("Unable to bind rep socket.");

        self.pub_socket
            .bind("tcp://0.0.0.0:5555")
            .expect("Unable to bind pub socket.");

        let mut msg = zmq::Message::new();

        loop {
            self.rep_socket
                .recv(&mut msg, 0)
                .expect("Unable to receive message.");

            let result: WorkerPacket = serde_json::from_str(msg.as_str().unwrap()).unwrap();
            info!("Got request from worker: {:?}", result);

            match result {
                WorkerPacket::Available => {
                    info!("Worker requested a job.");
                    let packet = ServerPacket::Work(self.current_work.clone());

                    info!("Sending work: {:?}", packet);
                    let work = serde_json::to_string(&packet).unwrap();

                    self.rep_socket
                        .send(&work, 0)
                        .expect("Unable to send message.")
                }
                WorkerPacket::Ready(work) => {
                    info!("Worker sent result of the job: {:?}", work);
                    match work.validate(&self.current_work) {
                        true => {
                            info!("Work has been validated.");
                            self.pub_socket
                                .send(
                                    &serde_json::to_string(&WorkDone(self.current_work.id()))
                                        .unwrap(),
                                    0,
                                )
                                .expect("Unable to send message.");
                            self.rep_socket
                                .send(&serde_json::to_string(&ServerPacket::Thanks).unwrap(), 0)
                                .expect("Unable to send message.");
                            self.generate_next();
                        }
                        false => {
                            info!("Work hasn't been validated.");
                            self.rep_socket
                                .send(&serde_json::to_string(&ServerPacket::Empty).unwrap(), 0)
                                .expect("Unable to send message.");
                        }
                    }
                }
            }
        }
    }

    fn generate_next(&mut self) {
        self.current_work = self.current_work.next();
    }
}

fn main() {
    let subscriber = FmtSubscriber::builder()
        .with_max_level(Level::TRACE)
        .finish();

    tracing::subscriber::set_global_default(subscriber).expect("setting default subscriber failed");

    let mut server = Server::new();

    info!("Starting server..");
    server.run();
}
