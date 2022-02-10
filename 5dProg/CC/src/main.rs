use std::cmp::{max, min};
use std::ops::{Add, Div, Mul, Rem};

use std::io;

#[derive(Debug, Clone)]
struct Polynomial {
    body: Vec<u8>,
}

impl Polynomial {
    fn new() -> Self {
        Polynomial { body: Vec::new() }
    }

    fn size(&self) -> usize {
        self.body.len()
    }

    fn set_size(&mut self, size: usize) {
        self.body.resize(size, 0);
    }

    fn push(&mut self, value: u8) {
        self.body.push(value);
    }

    fn last(&self) -> Option<&u8> {
        self.body.last()
    }

    fn insert(&mut self, value: u8) {
        self.body.insert(0, value);
    }

    fn pop(&mut self) -> Option<u8> {
        self.body.pop()
    }

    fn pop_count(&self) -> u8 {
        self.body.iter().fold(0, |acc, &x| acc + x)
    }

    fn print_binary(&self) {
        for x in self.body.iter() {
            match x {
                1 => print!("1"),
                0 => print!("0"),
                _ => panic!("Damn...."),
            }
        }
    }

    fn degree(&self) -> usize {
        self.size()
            - self
                .body
                .iter()
                .rev()
                .position(|&x| x == 1)
                .unwrap_or(self.size() - 1)
            - 1
    }

    fn move_right(&mut self, i: usize) {
        for _ in 0..i {
            self.body.insert(0, 0);
        }
    }
}

impl From<Vec<u8>> for Polynomial {
    fn from(body: Vec<u8>) -> Self {
        Polynomial { body }
    }
}

impl From<usize> for Polynomial {
    fn from(mut item: usize) -> Self {
        let mut new = Polynomial::new();

        while item != 0 {
            new.push((item & 1) as u8);
            item >>= 1;
        }

        new
    }
}

impl Add for Polynomial {
    type Output = Self;

    fn add(self, other: Self) -> Self {
        let mut new = Polynomial::new();
        for i in 0..max(self.size(), other.size()) {
            new.push(self.body.get(i).unwrap_or(&0) ^ other.body.get(i).unwrap_or(&0));
        }

        new
    }
}

impl PartialEq for Polynomial {
    fn eq(&self, other: &Self) -> bool {
        for i in 0..max(self.size(), other.size()) {
            if self.body.get(i).unwrap_or(&0) != other.body.get(i).unwrap_or(&0) {
                return false;
            }
        }

        true
    }
}

impl Mul for Polynomial {
    type Output = Self;

    fn mul(self, other: Self) -> Self {
        let degree = self.degree() + other.degree() + 1;
        let mut new = Polynomial::from(vec![0; degree]);

        for (i, &x) in other.body.iter().enumerate() {
            if x == 1 {
                let mut temp = self.clone();
                temp.move_right(i);
                new = new + temp;
            }
        }

        new
    }
}

impl Div for Polynomial {
    type Output = Self;

    fn div(mut self, div: Self) -> Self {
        if div.degree() > self.degree() {
            return Polynomial::from(vec![0]);
        }
        let mut ret = Polynomial::from(vec![0; self.degree() - div.degree() + 1]);
        while div.degree() <= self.degree() {
            let pow = self.degree() - div.degree();
            let mut temp = div.clone();
            temp.move_right(pow);
            ret.body[pow] = 1;
            self = self + temp;
        }

        ret
    }
}

impl Rem for Polynomial {
    type Output = Self;

    fn rem(mut self, div: Self) -> Self {
        if div.degree() > self.degree() {
            return self;
        }
        while div.degree() <= self.degree() {
            let pow = self.degree() - div.degree();
            let mut temp = div.clone();
            temp.move_right(pow);
            self = self + temp;
        }

        self
    }
}

impl std::fmt::Display for Polynomial {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let mut s = String::new();
        let mut fst = false;
        for (i, &x) in self.body.iter().enumerate() {
            if x == 1 {
                if fst {
                    s.push_str(" + ")
                }
                s.push_str(&format!("x^{}", i));
                fst = true;
            }
        }
        if s.is_empty() {
            s.push('0');
        }
        write!(f, "{} ", s)
    }
}

// TODO: Move to separate module
fn hamming_distance() -> u8 {
    let a = Polynomial::from(vec![1, 1, 1, 0, 1, 0, 1, 1, 1]);
    let mut m = 10;
    for i in (0..255).rev() {
        println!("{} {}", i, m);
        let f = Polynomial::from(i) * a.clone();
        for j in (0..255).rev() {
            if j != i {
                let s = Polynomial::from(j) * a.clone();
                let temp = m;
                m = min((f.clone() + s.clone()).pop_count(), m);
                if temp != m {
                    f.print_binary();
                    println!();
                    s.print_binary();
                    println!();
                    println!();
                }
            }
        }
    }
    m
}

struct Meggitt {
    message: Polynomial,
    register: Polynomial,
    buffer: Polynomial,
    generator: Polynomial,
    syndrom: Polynomial,
    output: Polynomial,
    fraction: Polynomial,
    tick: usize,
}

impl Meggitt {
    fn new(message: Polynomial, mut generator: Polynomial) -> Self {
        let register = Polynomial::from(vec![0; 4]);
        let buffer = Polynomial::from(vec![0; 15]); //asd
        let output = Polynomial::from(vec![0; 15]); //asd

        let mut syndrom = Polynomial::from(1 << message.size() - 1) % generator.clone();
        syndrom.set_size(4);
        syndrom.print_binary();
        print!("|");
        generator.set_size(4);
        generator.print_binary();
        println!("");

        let fraction = Polynomial::new();

        Meggitt {
            message,
            register,
            buffer,
            generator,
            syndrom,
            output,
            fraction,
            tick: 0,
        }
    }

    fn next(&mut self) {
        self.tick += 1;
        let mut out_bit = self.buffer.pop().unwrap_or(0);

        if self.register == self.syndrom {
            out_bit ^= 1;
        }

        let out_reg = self.register.pop().unwrap_or(0);
        let in_bit = self.message.pop().unwrap_or(0);

        self.buffer.insert(in_bit);
        self.register.insert(in_bit);
        self.register.set_size(4);

        if out_reg == 1 {
            self.register = self.register.clone() + self.generator.clone();
        }

        self.fraction.insert(out_reg);

        self.output.insert(out_bit);
        self.output.set_size(15);

        //println!(
        //    "\n{}: in:{}\tout:{}\tbuf:{}",
        //    30 - self.tick,
        //    in_bit,
        //    out_reg,
        //    out_bit
        //);
        //self.register.print_binary();
        //println!();
        //self.buffer.print_binary();
        //println!();
        //self.output.print_binary();
    }

    fn analyze_result(&self) {
        println!("Result of register: {}", self.register);
        println!("Result of fixing: {}", self.output);
        println!("Result of decoding: {}", self.fraction);
    }
}

fn decode() {
    let mut input_text = String::new();
    println!("Enter a number to decode: ");
    io::stdin()
        .read_line(&mut input_text)
        .expect("failed to read from stdin");

    let mut input_poly = Polynomial::new();

    for c in input_text.trim().chars().rev() {
        let num = match c {
            '1' => 1,
            '0' => 0,
            _ => panic!("Invalid data!"),
        };

        input_poly.insert(num);
    }
    input_poly.set_size(15);
    println!("Polynomial: {}", input_poly);
    let generator = Polynomial::from(vec![1, 0, 0, 1, 1]);
    let mut decoder = Meggitt::new(input_poly, generator.clone());

    for _ in 0..15 {
        decoder.next();
    }
    //decoder.analyze_result();

    for _ in 0..15 {
        decoder.next();
    }
    decoder.analyze_result();
    let decoded = decoder.output.clone() / generator;
    println!("Decoded polynom: {}", decoded);
    println!("In binary: ");
    decoded.print_binary();
}

fn encode() {
    let mut input_text = String::new();
    println!("Enter a number to encode: ");
    io::stdin()
        .read_line(&mut input_text)
        .expect("failed to read from stdin");

    let mut input_poly = Polynomial::new();
    input_poly.set_size(11);

    for c in input_text.trim().chars().rev() {
        let num = match c {
            '1' => 1,
            '0' => 0,
            _ => panic!("Invalid data!"),
        };

        input_poly.insert(num);
    }
    println!("Polynomial: {}", input_poly);

    let generator = Polynomial::from(vec![1, 0, 0, 1, 1]);
    let encoded = generator * input_poly;
    println!("Encoded polynomial: {}", encoded);
    print!("Binary form: ");
    for x in encoded.body {
        match x {
            1 => print!("1"),
            0 => print!("0"),
            _ => panic!("Damn...."),
        }
    }
}

fn main() {
    //let mut input_text = String::new();
    //println!("Do you want to encode or decode?[e/d]: ");
    //io::stdin()
    //    .read_line(&mut input_text)
    //    .expect("failed to read from stdin");

    //match input_text.trim() {
    //    "e" => encode(),
    //    "d" => decode(),
    //    _ => panic!("Invalid input"),
    //}

    //println!("{}", hamming_distance());
    let generator = Polynomial::from(vec![1, 1, 1, 0, 1, 0, 1, 1, 1]);
    for i in 10..40 {
        println!("\n{}-----------\n", i);
        let mut kek = generator.clone();
        kek.set_size(i);
        for x in 0..i {
            let a = kek.pop().unwrap();
            kek.insert(a);
            println!("{} {}", kek.clone(), kek.clone() % generator.clone());
        }
    }
}
