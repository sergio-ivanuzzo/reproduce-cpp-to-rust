use std::ffi::{c_char, CString};
use std::fs::read_to_string;
use yaml_rust::{Yaml, YamlLoader};

#[link(name="srp6")]
extern "C" {
    fn test_srp6(
        N_str: *const c_char,
        g_str: *const c_char,
        B_str: *const c_char,
        s_str: *const c_char,
        username_str: *const c_char,
        password_str: *const c_char
    );
}

const CONFIG_CONTENT: &str = r##"connection_data:
  127.0.0.1:
    username:
        password: "safe_password"
"##;

#[derive(Clone, Debug)]
pub struct Data {
    pub account: String,
    pub password: String,
}

fn parse_data(config: &Yaml) -> Data {
    let config = &config["username"];
    let account = "username".to_uppercase();
    let password = config["password"].as_str().unwrap().to_string().to_uppercase();

    Data { account, password }
}

fn main() {
    println!("Hello, world!");

    let docs = YamlLoader::load_from_str(CONFIG_CONTENT).unwrap();
    let connection_data = parse_data(&docs[0]["connection_data"]["127.0.0.1"]);

    let n: [u8; 32] = rand::random();
    let g: Vec<u8> = vec![7];
    let server_ephemeral: [u8; 32] = rand::random();
    let salt: [u8; 32] = rand::random();
    let account = String::from("BOT1");
    let password = String::from("password");
    let account_str = CString::new(account).expect("Failed to convert account to CString");
    let password_str = CString::new(password).expect("Failed to convert password to CString");

    println!("FIRST");
    unsafe {
        test_srp6(
            n.as_ptr() as *const c_char,
            g.as_ptr() as *const c_char,
            server_ephemeral.as_ptr() as *const c_char,
            salt.as_ptr() as *const c_char,
            account_str.as_ptr() as *const c_char,
            password_str.as_ptr() as *const c_char
        );
    }

    let n: [u8; 32] = rand::random();
    let g: Vec<u8> = vec![7];
    let server_ephemeral: [u8; 32] = rand::random();
    let salt: [u8; 32] = rand::random();
    let account = connection_data.account;
    let password = connection_data.password;

    // println!("SECOND");
    unsafe {
        test_srp6(
            n.as_ptr() as *const c_char,
            g.as_ptr() as *const c_char,
            server_ephemeral.as_ptr() as *const c_char,
            salt.as_ptr() as *const c_char,
            account_str.as_ptr() as *const c_char,
            password_str.as_ptr() as *const c_char
        );
    }
}
