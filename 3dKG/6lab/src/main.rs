use image::{Rgb, RgbImage};
use imageproc::drawing::{
    draw_hollow_circle_mut, draw_cubic_bezier_curve_mut, draw_filled_circle_mut, draw_filled_rect_mut,
};
use imageproc::rect::Rect;
use std::path::Path;

/* Отрисовка флага РФ */
fn russ_flag() {
    let mut image = RgbImage::new(400, 400);
    let path = Path::new("russ_flag.png");

    let red = Rgb([255u8, 0u8, 0u8]);
    let blue = Rgb([0u8, 0u8, 255u8]);
    let white = Rgb([255u8, 255u8, 255u8]);

    draw_filled_rect_mut(&mut image, Rect::at(100, 100).of_size(200, 33), white);
    draw_filled_rect_mut(&mut image, Rect::at(100, 133).of_size(200, 33), blue);
    draw_filled_rect_mut(&mut image, Rect::at(100, 166).of_size(200, 33), red);

    image.save(path).expect("Error saving image");
}

fn belg_flag() {
    let mut image = RgbImage::new(400, 400);
    let path = Path::new("belg_flag.png");

    let red = Rgb([255u8, 0u8, 0u8]);
    let yellow = Rgb([255u8, 255u8, 0u8]);
    let black = Rgb([10u8, 10u8, 10u8]);

    draw_filled_rect_mut(&mut image, Rect::at(100, 100).of_size(66, 100), black);
    draw_filled_rect_mut(&mut image, Rect::at(166, 100).of_size(66, 100), yellow);
    draw_filled_rect_mut(&mut image, Rect::at(232, 100).of_size(66, 100), red);

    image.save(path).expect("Error saving image");
}

fn face_no_smile() {
    let mut image = RgbImage::new(400, 400);
    let path = Path::new("face_no_smile.png");

    let red = Rgb([255u8, 0u8, 0u8]);
    let yellow = Rgb([255u8, 255u8, 0u8]);
    let blue = Rgb([0u8, 0u8, 255u8]);

    draw_filled_circle_mut(&mut image, (200, 200), 100, yellow);

    for x in 0..5 {
        draw_cubic_bezier_curve_mut(
            &mut image,
            (160.0, 240.0 + x as f32),
            (240.0, 240.0 + x as f32),
            (160.0, 200.0 + x as f32),
            (240.0, 200.0 + x as f32),
            red,
        );
        draw_hollow_circle_mut(&mut image, (160, 160), 10+x, blue);
        draw_hollow_circle_mut(&mut image, (240, 160), 10+x, blue);
    }

    image.save(path).expect("Error saving image");
}

fn face_smile() {
    let mut image = RgbImage::new(400, 400);
    let path = Path::new("face_smile.png");

    let red = Rgb([255u8, 0u8, 0u8]);
    let yellow = Rgb([255u8, 255u8, 0u8]);
    let blue = Rgb([0u8, 0u8, 255u8]);

    draw_filled_circle_mut(&mut image, (200, 200), 100, yellow);

    for x in 0..5 {
        draw_cubic_bezier_curve_mut(
            &mut image,
            (160.0, 200.0 + x as f32),
            (240.0, 200.0 + x as f32),
            (160.0, 240.0 + x as f32),
            (240.0, 240.0 + x as f32),
            red,
        );
        draw_hollow_circle_mut(&mut image, (160, 160), 10+x, blue);
        draw_hollow_circle_mut(&mut image, (240, 160), 10+x, blue);
    }

    image.save(path).expect("Error saving image");
}

fn face_wink() {
    let mut image = RgbImage::new(400, 400);
    let path = Path::new("face_wink.png");

    let red = Rgb([255u8, 0u8, 0u8]);
    let yellow = Rgb([255u8, 255u8, 0u8]);
    let blue = Rgb([0u8, 0u8, 255u8]);

    draw_filled_circle_mut(&mut image, (200, 200), 100, yellow);

    for x in 0..5 {
        draw_cubic_bezier_curve_mut(
            &mut image,
            (160.0, 200.0 + x as f32),
            (240.0, 200.0 + x as f32),
            (160.0, 240.0 + x as f32),
            (240.0, 240.0 + x as f32),
            red,
        );
        draw_hollow_circle_mut(&mut image, (160, 160), 10+x, blue);
    }
    draw_filled_rect_mut(&mut image, Rect::at(230, 155).of_size(30, 5), blue);

    image.save(path).expect("Error saving image");
}

fn main() {
    russ_flag();
    belg_flag();
    face_no_smile();
    face_smile();
    face_wink();


    println!("Hello, world!");
}
