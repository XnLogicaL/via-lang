#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>

const auto root_path = "C:/.via"; // Destination directory
const auto this_path = std::filesystem::current_path();
const auto root_files = this_path / "src_new/via";  // Source directory

int main()
{
    using namespace std::filesystem;

    try {
        // Check if the source directory exists
        if (!exists(root_files)) {
            std::cerr << "Source directory " << root_files << " does not exist.\n";
            return 1;
        }

        // Check if the destination directory exists
        if (exists(root_path)) {
            // Remove the destination directory recursively
            remove_all(root_path);
        }

        // Ensure the parent directory exists before copying
        create_directories(path(root_path).parent_path());

        // Copy the source files to the destination, recursively
        copy(root_files, root_path, copy_options::recursive);

        

        std::cout << "Files copied successfully to " << root_path << "\n";
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
            return 1;
    }

    return 0;
}