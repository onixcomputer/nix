extern "C" {
    fn panic(ptr: *const u8, len: usize) -> !;
}

fn main() {
    unsafe {
        let msg = b"wasi intentional panic for testing";
        panic(msg.as_ptr(), msg.len());
    }
}
