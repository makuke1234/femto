use std::io;
use std::io::Write;

fn main()
{
	print!("Enter number of slots in LUT: ");
	io::stdout().flush().unwrap();
	
	let mut input = String::new();
	io::stdin().read_line(&mut input).expect("Failed to read line!");
	
	// Convert string to int
	let n: usize = input.trim().parse::<usize>().unwrap();
	
	// Create LUT vector
	let mut vector: Vec<f32> = Vec::with_capacity(n);
	println!("Created vector of floats with size of {}!", n);
	
	let n_f: f32 = 2.0 * (n as f32);
	for i in 0..n
	{
		vector.push((2.0 * 3.1415926535 * (i as f32) / n_f).cos());
	}
	println!("Vector filled!\nContents:");
	for x in &vector
	{
		print!("{:.3}, ", x);
	}
	println!();	// New-line at the end
}
