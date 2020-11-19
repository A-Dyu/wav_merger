#pragma once
#include <iostream>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

struct wav_file {
  wav_file(const char*, const char*);

  wav_file(const char*, const char*, std::vector<wav_file> const&, double);

  ~wav_file();

  uint32_t get_data_size() const noexcept;

  void set_data_size(uint32_t) noexcept;

  uint16_t get_num_channels() const noexcept;

  void set_num_channels(uint16_t) noexcept;

  uint32_t get_sample_rate() const noexcept;

  void set_sample_rate(uint32_t) noexcept;

  uint32_t get_byte_rate() const noexcept;

  void set_byte_rate(uint32_t) noexcept;

  uint32_t get_bits_per_sample() const noexcept;\

  uint32_t get_chunk_size() const noexcept;

private:
  static constexpr size_t HEADER_SIZE = 44;
  static constexpr uint32_t CHUNK_SIZE = 4;
  static constexpr uint32_t SUBCHUNK_SIZE = 16;
  static constexpr uint16_t NUM_CHANNELS = 22;
  static constexpr uint32_t SAMPLE_RATE = 24;
  static constexpr uint32_t BYTE_RATE = 28;
  static constexpr uint16_t BITS_PER_SAMPLE = 34;
  static constexpr uint32_t DATA_SIZE = 40;

  template<typename IntT>
  IntT& header_cast(IntT shift) noexcept {
    return *reinterpret_cast<IntT*>(header + shift);
  }

  template<typename IntT>
  IntT const& header_cast(IntT shift) const noexcept {
    return *reinterpret_cast<const IntT*>(header + shift);
  }

  template<typename IntT>
  void write_merged_data(std::vector<wav_file> const& mono_files, uint32_t max_data_size, double amp_multiplier) {
    size_t block_size = get_bits_per_sample() / 8;
    for (uint32_t i = 0; i < max_data_size; i++) {
      for (auto const& mono_file : mono_files) {
        IntT buf;
        if (i < mono_file.get_data_size()) {
          std::fread(&buf, 1, block_size, mono_file.file);
        } else {
          buf = 0;
        }
        buf *= amp_multiplier;
        std::fwrite(&buf, 1, block_size, file);
      }
    }
  }

  u_int32_t get_subchunk_size() const noexcept;

  void set_bits_per_sample(uint32_t) noexcept;

  void set_chunk_size(uint32_t) noexcept;

  void write_header();

  bool is_writable() const noexcept;

  std::FILE* file;
  char header[HEADER_SIZE];
  const char* mode;
};

bool are_mergeable(wav_file const&, wav_file const&);


