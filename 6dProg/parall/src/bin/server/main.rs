use serde_json::json;
use std::thread;
use std::time::Duration;
use zmq;

fn validator(p: usize, q: usize) {
    let p_range = vec![1; p];
    let q_range = vec![0; q];

    let ctx = zmq::Context::new();

    let socket = ctx.socket(zmq::PUSH).expect("Unable to create socket.");
    socket
        .bind("tcp://0.0.0.0:5557")
        .expect("Unable to bind socket.");
    thread::sleep(Duration::from_secs(1));

    for p in p_range {
        for q in &q_range {
            let work = json!({
                "p": p,
                "q": q,

            });
            socket
                .send(&work.to_string(), 0)
                .expect("Unable to send message.");
            println!("Sent p: {}; q: {}", p, q);
        }
    }
}

fn sink(p: usize, q: usize) {
    let total = p * q;

    let ctx = zmq::Context::new();

    let socket = ctx.socket(zmq::PULL).expect("Unable to create socket.");
    socket
        .bind("tcp://0.0.0.0:5558")
        .expect("Unable to bind socket.");

    let mut processed = 0;
    let mut msg = zmq::Message::new();

    while processed < total {
        socket
            .recv(&mut msg, 0)
            .expect("Unable to receive message.");
        let result = msg.as_str().unwrap().parse::<i32>().unwrap();
        println!("<{}> Got result: {}", processed, result);
        processed += 1;
    }
}

fn main() {
    let p = 5;
    let q = 5;

    let sink = thread::spawn(move || {
        sink(p, q);
    });
    println!("Started sink.");

    let validator = thread::spawn(move || {
        validator(p, q);
    });
    println!("Started validator.");

    sink.join().unwrap();
    validator.join().unwrap();
}
