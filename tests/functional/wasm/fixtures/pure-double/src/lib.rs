extern "C" {
    fn get_int(value: u32) -> i64;
    fn make_int(n: i64) -> u32;
    fn warn(ptr: *const u8, len: usize);
}

#[no_mangle]
pub extern "C" fn nix_wasm_init_v1() {}

#[no_mangle]
pub extern "C" fn double(arg: u32) -> u32 {
    unsafe {
        let msg = b"doubling";
        warn(msg.as_ptr(), msg.len());
        let n = get_int(arg);
        make_int(n * 2)
    }
}
