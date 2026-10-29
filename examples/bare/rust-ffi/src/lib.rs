//! no_std Rust called from C (and calling back into C): a Fletcher-16 checksum, an in-place insertion sort and a
//! callback that prints through the C console. Panics end the run with RESULT: FAIL.
#![no_std]

use core::panic::PanicInfo;

extern "C" {
    fn bare_puts(s: *const u8);
    fn chip_exit(code: i32) -> !;
}

#[no_mangle]
pub extern "C" fn rust_fletcher16(data: *const u8, len: usize) -> u16 {
    let bytes = unsafe { core::slice::from_raw_parts(data, len) };
    let (mut a, mut b) = (0u16, 0u16);
    for &x in bytes {
        a = (a + x as u16) % 255;
        b = (b + a) % 255;
    }
    (b << 8) | a
}

#[no_mangle]
pub extern "C" fn rust_sort(data: *mut i32, len: usize) {
    let v = unsafe { core::slice::from_raw_parts_mut(data, len) };
    v.sort_unstable();
}

#[no_mangle]
pub extern "C" fn rust_hello() {
    unsafe { bare_puts(b"hello from Rust (no_std)\n\0".as_ptr()) };
}

#[panic_handler]
fn panic(_: &PanicInfo) -> ! {
    unsafe {
        bare_puts(b"Rust panic\nRESULT: FAIL (panic)\n\0".as_ptr());
        chip_exit(1)
    }
}
