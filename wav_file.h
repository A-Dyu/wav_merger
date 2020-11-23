#pragma once
#include <iostream>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

struct wav_file_header {
  uint16_t n_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
  uint32_t data_size;
};

struct wav_file : wav_file_header {
  wav_file(const char*, const char*);

  wav_file(const char*, const char*, std::vector<wav_file> const&, double);

  wav_file(wav_file const&) = delete;

  wav_file(wav_file&& other) noexcept;

  wav_file& operator=(wav_file const&) = delete;

  wav_file& operator=(wav_file&& other);

  ~wav_file();

  uint32_t get_data_size() const noexcept;

  uint16_t get_num_channels() const noexcept;

  uint32_t get_sample_rate() const noexcept;

  uint32_t get_byte_rate() const noexcept;

  uint32_t get_bits_per_sample() const noexcept;\


private:
  template<typename IntT>
  void write_merged_data(std::vector<wav_file> const& mono_files, uint32_t max_data_size, double amp_multiplier) {
    for (uint32_t i = 0; i < max_data_size; i++) {
      size_t l_channels = 0, r_channels = 0;
      int64_t l_val = 0, r_val = 0;
      for (size_t j = 0; j < mono_files.size() / 2; j++)
        if (mono_files[j].data_size > i * sizeof(IntT)) {
          l_val += mono_files[j].read_number<IntT>();
          l_channels++;
        }
      for (size_t j = mono_files.size() / 2; j < mono_files.size(); j++)
        if (mono_files[j].data_size > i * sizeof(IntT)) {
          r_val += mono_files[j].read_number<IntT>();
          r_channels++;
        }
      l_val /= std::max(l_channels, static_cast<size_t>(1));
      r_val /= std::max(r_channels, static_cast<size_t>(1));
      l_val *= amp_multiplier;
      r_val *= amp_multiplier;
      write_number<IntT>(l_val);
      write_number<IntT>(r_val);
    }
  }

  void write_header();

  void write_chunk_id(const char*);

  template<typename IntT>
  void write_number(IntT number) {
    std::fwrite(&number, 1, sizeof(number), file);
  }

  std::string read_chunk_header() const;

  template<typename IntT>
  IntT read_number() const {
      char n_data[sizeof(IntT)];
      read_n_bytes(n_data, sizeof(IntT));
      return *reinterpret_cast<IntT*>(n_data);
  }

  void read_n_bytes(char* dst, size_t n) const;

  void check_chunk_id(std::string const& id) const;

  bool is_writable() const noexcept;

  std::FILE* file;
  const char* mode;
};

bool are_mergeable(wav_file const&, wav_file const&);


