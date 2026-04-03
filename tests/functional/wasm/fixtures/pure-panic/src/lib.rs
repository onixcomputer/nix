extern "C" {
    fn panic(ptr: *const u8, len: usize) -> !;
}

#[no_mangle]
pub extern "C" fn nix_wasm_init_v1() {}

#[no_mangle]
pub extern "C" fn will_panic(_arg: u32) -> u32 {
    unsafe {
        let msg = b"intentional panic for testing";
        panic(msg.as_ptr(), msg.len());
    }
}
