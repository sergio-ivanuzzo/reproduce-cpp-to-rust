use std::ffi::c_char;

extern "C" {
    #[link(name="libsrp6", kind="dylib")]
    fn test_srp6(
        N_str: *const c_char,
        g_str: *const c_char,
        B_str: *const c_char,
        s_str: *const c_char,
        username_str: *const c_char,
        password_str: *const c_char
    );
}

fn main() {
    println!("Hello, world!");

    let n: [u8; 32] = rand::random();
    let g: Vec<u8> = vec![7];
    let server_ephemeral: [u8; 32] = rand::random();
    let salt: [u8; 32] = rand::random();
    let account = String::from("ACCOUNT");
    let password = String::from("PASSWORD");

    unsafe {
        test_srp6(
            n.as_ptr() as *const c_char,
            g.as_ptr() as *const c_char,
            server_ephemeral.as_ptr() as *const c_char,
            salt.as_ptr() as *const c_char,
            account.as_ptr() as *const c_char,
            password.as_ptr() as *const c_char
        );
    }
}
