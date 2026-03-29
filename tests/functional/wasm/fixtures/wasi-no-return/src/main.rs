/// WASI module that exits without calling return_to_nix.
/// Used to test the "finished without returning a value" error path.
fn main() {
    let _args: Vec<String> = std::env::args().collect();
    // Deliberately do nothing with the argument and never call return_to_nix.
}
