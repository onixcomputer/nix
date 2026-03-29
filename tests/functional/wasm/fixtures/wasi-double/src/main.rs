extern "C" {
    fn get_int(value: u32) -> i64;
    fn make_int(n: i64) -> u32;
    fn return_to_nix(value: u32) -> !;
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let arg_id: u32 = args[1].parse().expect("argv[1] should be a ValueId");
    unsafe {
        let n = get_int(arg_id);
        let result = make_int(n * 2);
        return_to_nix(result);
    }
}
