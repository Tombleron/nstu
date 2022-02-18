use parall::utils::{State, Work};
use std::env;
use std::thread;

fn worker() {
    let ctx = zmq::Context::new();

    let socket = ctx.socket(zmq::REQ).expect("Unable to create socket.");

    socket
        .connect("tcp://127.0.0.1:5557")
        .expect("Unable to connect to validator socket.");

    let mut msg = zmq::Message::new();

    loop {
        socket
            .send(&serde_json::to_string(&State::Availabe).unwrap(), 0)
            .expect("Unable to send message.");

        socket
            .recv(&mut msg, 0)
            .expect("Unable to retrive message.");

        let result: Work = serde_json::from_str(msg.as_str().unwrap()).unwrap();

        println!("Got work: {:?}", result);

        match result {
            Work::Thanks => {}
            Work::Empty => {}
            Work::Work((p, q)) => {
                socket
                    .send(&serde_json::to_string(&State::Ready(p + q)).unwrap(), 0)
                    .expect("Unable to send message.");
                socket
                    .recv(&mut msg, 0)
                    .expect("Unable to retrive message.");
            }
        }
    }
}

fn main() {
    if env::args().len() < 2 {
        println!("Usage: {} <workers_count>", env::args().last().unwrap());
    } else {
        let mut handlers = Vec::new();
        let count = env::args().last().unwrap().parse::<usize>().unwrap();
        for _ in 0..count {
            handlers.push(thread::spawn(worker));
        }

        for handler in handlers {
            handler.join().unwrap();
        }
    }
}
