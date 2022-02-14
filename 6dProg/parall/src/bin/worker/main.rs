use std::env;
use std::thread;
use serde::Deserialize;
use serde::Serialize;
use serde_json;
use zmq;

#[derive(Deserialize, Serialize, Debug)]
struct Data {
    p: i32,
    q: i32,
}

fn worker() {

    let ctx = zmq::Context::new();

    let socket_out= ctx.socket(zmq::PUSH).expect("Unable to create socket.");
    let socket_in= ctx.socket(zmq::PULL).expect("Unable to create socket.");

    socket_in.connect("tcp://127.0.0.1:5557")
        .expect("Unable to connect to validator socket.");
    socket_out.connect("tcp://127.0.0.1:5558")
        .expect("Unable to connect to sink socket.");

    let mut msg = zmq::Message::new();

    loop {

        socket_in.recv(&mut msg, 0).expect("Unable to receive message.");

        let result: Data = serde_json::from_str(msg.as_str().unwrap()).unwrap();
        println!("Got result: {:?}", result);

        socket_out.send(&result.p.to_string(), 0).expect("Unable to send message.");

    }
}
fn main() {
    if env::args().len() < 2 {
        println!("Usage: {} <workers_count>", env::args().last().unwrap());
    } else {
        let mut handlers = Vec::new();
        let count = env::args().last().unwrap().parse::<usize>().unwrap();
        for _ in 0..count {
            handlers.push(thread::spawn(|| worker()));
        }

        for handler in handlers {
            handler.join().unwrap();
        }
    }
}
