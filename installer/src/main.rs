use std::{fs, path::Path, process::Command};

fn get_binary_url() -> &'static str {
    #[cfg(all(target_os = "windows", target_arch = "x86_64"))]
    {
        "https://github.com/XnLogicaL/via-lang/releases/latest/download/via-x86_64-windows.exe"
    }

    #[cfg(all(target_os = "linux", target_arch = "x86_64"))]
    {
        "https://github.com/XnLogicaL/via-lang/releases/latest/download/via-x86_64-linux"
    }

    #[cfg(all(target_os = "macos", target_arch = "x86_64"))]
    {
        "https://github.com/XnLogicaL/via-lang/releases/latest/download/via-x86_64-osx"
    }

    #[cfg(not(any(
        all(target_os = "windows", target_arch = "x86_64"),
        all(target_os = "linux", target_arch = "x86_64"),
        all(target_os = "macos", target_arch = "x86_64")
    )))]
    {
        compile_error!("Unsupported platform or architecture");
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Default install path for Windows
    #[cfg(target_os = "windows")]
    let install_dir = Path::new(r"C:\Program Files\via");

    #[cfg(target_os = "linux")]
    let install_dir = Path::new("/usr/lib/via");

    // Create directory (may require admin on Windows)
    fs::create_dir_all(install_dir)?;

    // Download your prebuilt binary
    let url = get_binary_url();
    let bin_name = if cfg!(target_os = "windows") {
        "via.exe"
    } else {
        "via"
    };

    let bin_path = install_dir.join(bin_name);
    let response = reqwest::blocking::get(url)?.bytes()?;
    fs::write(&bin_path, &response)?;

    #[cfg(not(target_os = "windows"))]
    {
        use std::os::unix::fs::PermissionsExt;
        fs::set_permissions(&bin_path, std::fs::Permissions::from_mode(0o755))?;
    }

    println!("Installed to {:?}", install_dir);
    Ok(())
}
