use parall::utils::{State, Work};

fn server(p: usize, q: usize) {
    let mut works = Vec::with_capacity(p * q);
    for i in 0..p {
        for j in 0..q {
            works.push((i, j));
        }
    }

    let ctx = zmq::Context::new();

    let socket = ctx.socket(zmq::REP).expect("Unable to create socket.");
    socket
        .bind("tcp://0.0.0.0:5557")
        .expect("Unable to bind socket.");

    let mut results: Vec<usize> = Vec::with_capacity(p * q);
    let mut msg = zmq::Message::new();

    while results.len() < p * q {
        socket
            .recv(&mut msg, 0)
            .expect("Unable to receive message.");

        let result: State = serde_json::from_str(msg.as_str().unwrap()).unwrap();

        match result {
            State::Availabe => {
                if !works.is_empty() {
                    let data = works.pop().unwrap();

                    let work = serde_json::to_string(&Work::Work((data.0, data.1))).unwrap();

                    socket.send(&work, 0).expect("Unable to send message.")
                } else {
                    socket
                        .send(&serde_json::to_string(&Work::Empty).unwrap(), 0)
                        .expect("Unable to send message.")
                }
            }
            State::Ready(value) => {
                println!("Got result {}: {}", results.len(), value);

                results.push(value);

                socket
                    .send(&serde_json::to_string(&Work::Thanks).unwrap(), 0)
                    .expect("Unable to send message.")
            }
        }
    }
}

fn main() {
    let p = 5;
    let q = 5;

    server(p, q);
}
