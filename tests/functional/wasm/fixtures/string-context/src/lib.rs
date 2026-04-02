extern "C" {
    fn copy_string(value: u32, ptr: *mut u8, max_len: u32) -> u32;
    fn make_string(ptr: *const u8, len: u32) -> u32;
    fn has_context(value: u32) -> i32;
    fn get_string_context_count(value: u32) -> u32;
    fn copy_string_context(value: u32, idx: u32, ptr: *mut u8, max_len: u32) -> u32;
    fn make_string_with_context(
        str_ptr: *const u8,
        str_len: u32,
        ctx_ptr: *const [u32; 2],
        ctx_count: u32,
    ) -> u32;
    fn make_int(n: i64) -> u32;
    fn make_attrset(ptr: *const [u32; 3], len: u32) -> u32;
}

#[no_mangle]
pub extern "C" fn nix_wasm_init_v1() {}

/// Accepts a string, returns an attrset with:
///   has_ctx: bool (int 0/1) — whether the string had context
///   ctx_count: int — number of context elements
///   value: string — the string content (recreated with make_string, no context)
///   round_trip: string — the string recreated with make_string_with_context (preserving context)
#[no_mangle]
pub extern "C" fn inspect_context(arg: u32) -> u32 {
    unsafe {
        // Read has_context
        let has_ctx = has_context(arg);
        let has_ctx_val = make_int(has_ctx as i64);

        // Read context count
        let ctx_count = get_string_context_count(arg);
        let ctx_count_val = make_int(ctx_count as i64);

        // Read the string content
        let mut str_buf = [0u8; 4096];
        let str_len = copy_string(arg, str_buf.as_mut_ptr(), str_buf.len() as u32);
        let str_bytes = &str_buf[..str_len as usize];

        // Read all context elements
        let mut ctx_elems: Vec<Vec<u8>> = Vec::new();
        for i in 0..ctx_count {
            let mut ctx_buf = [0u8; 512];
            let elem_len =
                copy_string_context(arg, i, ctx_buf.as_mut_ptr(), ctx_buf.len() as u32);
            ctx_elems.push(ctx_buf[..elem_len as usize].to_vec());
        }

        // Recreate without context (plain make_string)
        let plain_val = make_string(str_bytes.as_ptr(), str_len);

        // Recreate with context (make_string_with_context)
        let ctx_entries: Vec<[u32; 2]> = ctx_elems
            .iter()
            .map(|e| [e.as_ptr() as u32, e.len() as u32])
            .collect();
        let round_trip_val = make_string_with_context(
            str_bytes.as_ptr(),
            str_len,
            ctx_entries.as_ptr(),
            ctx_entries.len() as u32,
        );

        // Build result attrset
        let key_has_ctx = b"has_ctx";
        let key_ctx_count = b"ctx_count";
        let key_value = b"value";
        let key_round_trip = b"round_trip";

        let attrs: [[u32; 3]; 4] = [
            [key_has_ctx.as_ptr() as u32, key_has_ctx.len() as u32, has_ctx_val],
            [key_ctx_count.as_ptr() as u32, key_ctx_count.len() as u32, ctx_count_val],
            [key_value.as_ptr() as u32, key_value.len() as u32, plain_val],
            [key_round_trip.as_ptr() as u32, key_round_trip.len() as u32, round_trip_val],
        ];

        make_attrset(attrs.as_ptr(), 4)
    }
}

/// Accepts any string, returns it unchanged via make_string_with_context
/// (identity round-trip that preserves context).
#[no_mangle]
pub extern "C" fn passthrough_string(arg: u32) -> u32 {
    unsafe {
        // Read string content
        let mut str_buf = [0u8; 4096];
        let str_len = copy_string(arg, str_buf.as_mut_ptr(), str_buf.len() as u32);

        // Read context
        let ctx_count = get_string_context_count(arg);
        let mut ctx_elems: Vec<Vec<u8>> = Vec::new();
        for i in 0..ctx_count {
            let mut ctx_buf = [0u8; 512];
            let elem_len =
                copy_string_context(arg, i, ctx_buf.as_mut_ptr(), ctx_buf.len() as u32);
            ctx_elems.push(ctx_buf[..elem_len as usize].to_vec());
        }

        // Recreate with context
        let ctx_entries: Vec<[u32; 2]> = ctx_elems
            .iter()
            .map(|e| [e.as_ptr() as u32, e.len() as u32])
            .collect();
        make_string_with_context(
            str_buf.as_ptr(),
            str_len,
            ctx_entries.as_ptr(),
            ctx_entries.len() as u32,
        )
    }
}
