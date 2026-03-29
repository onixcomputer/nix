extern "C" {
    fn make_string(ptr: *const u8, len: usize) -> u32;
    fn return_to_nix(value: u32) -> !;
}

fn main() {
    println!("hello from stdout");
    eprintln!("hello from stderr");

    let args: Vec<String> = std::env::args().collect();
    let _arg_id: u32 = args[1].parse().expect("argv[1] should be a ValueId");

    unsafe {
        let msg = b"done";
        let result = make_string(msg.as_ptr(), msg.len());
        return_to_nix(result);
    }
}
