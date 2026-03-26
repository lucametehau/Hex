#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <string>
#include <filesystem>
#include <array>
#include <chrono>

// Replace this with your actual HexBoard struct if you prefer, 
// but 172 bytes is the exact size of your Rust HexPosition!
constexpr std::size_t RECORD_SIZE = 172; 
using Record = std::array<char, RECORD_SIZE>;

constexpr int NUM_BINS = 16; // Splits memory usage by 16x

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: ./shuffle <input.bin> <output.bin>\n";
        return 1;
    }

    std::string input_path = argv[1];
    std::string output_path = argv[2];

    std::mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> bin_dist(0, NUM_BINS - 1);

    std::cout << "Step 1: Scattering data into " << NUM_BINS << " temporary bins...\n";
    
    std::ifstream in_file(input_path, std::ios::binary);
    if (!in_file) {
        std::cerr << "Failed to open input file.\n";
        return 1;
    }

    // Open 16 temporary files for writing
    std::vector<std::ofstream> bin_files;
    for (int i = 0; i < NUM_BINS; ++i) {
        bin_files.emplace_back("temp_bin_" + std::to_string(i) + ".bin", std::ios::binary);
    }

    // Read in chunks to keep disk I/O fast (e.g., 8192 records at a time)
    constexpr std::size_t CHUNK_SIZE = 8192;
    std::vector<Record> read_buffer(CHUNK_SIZE);

    std::size_t total_records = 0;
    while (in_file) {
        in_file.read(reinterpret_cast<char*>(read_buffer.data()), CHUNK_SIZE * RECORD_SIZE);
        std::size_t records_read = in_file.gcount() / RECORD_SIZE;

        for (std::size_t i = 0; i < records_read; ++i) {
            int target_bin = bin_dist(rng);
            bin_files[target_bin].write(read_buffer[i].data(), RECORD_SIZE);
            total_records++;
        }
    }
    in_file.close();

    for (auto& bf : bin_files) bf.close();
    std::cout << "Scattered " << total_records << " positions successfully.\n";

    std::cout << "Step 2: Local shuffling and merging...\n";
    std::ofstream out_file(output_path, std::ios::binary);

    for (int i = 0; i < NUM_BINS; ++i) {
        std::string bin_name = "temp_bin_" + std::to_string(i) + ".bin";
        std::ifstream bin_in(bin_name, std::ios::binary | std::ios::ate);
        
        if (!bin_in) continue;

        // Load the entire small bin into RAM
        std::streamsize bin_bytes = bin_in.tellg();
        std::size_t bin_records = bin_bytes / RECORD_SIZE;
        bin_in.seekg(0, std::ios::beg);

        std::vector<Record> bin_data(bin_records);
        bin_in.read(reinterpret_cast<char*>(bin_data.data()), bin_bytes);
        bin_in.close();

        // Mathematically perfect shuffle in RAM
        std::shuffle(bin_data.begin(), bin_data.end(), rng);

        // Dump to final output
        out_file.write(reinterpret_cast<const char*>(bin_data.data()), bin_bytes);

        // Delete the temp file so we don't litter your hard drive
        std::filesystem::remove(bin_name);
        
        std::cout << "Merged bin " << i + 1 << "/" << NUM_BINS << "...\r";
    }

    out_file.close();
    std::cout << "\nDone! Fully shuffled " << output_path << ".\n";

    return 0;
}