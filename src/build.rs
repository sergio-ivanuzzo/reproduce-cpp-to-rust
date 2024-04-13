fn main() {
    let dst = cmake::build("libsrp6");

    println!("cargo:rustc-link-search=native={}", dst.display());
    println!("cargo:rustc-link-search=libsrp6");
    println!("cargo:rustc-link-lib=dylib=srp6");
    println!("cargo:rustc-link-lib=dylib=ssl");
    println!("cargo:rustc-link-lib=dylib=crypto");
    println!("cargo:rustc-link-lib=dylib=stdc++");
}
