#include "wav_file.h"
wav_file::wav_file(const char *file_name, const char *mode): mode(mode) {
  file = std::fopen(file_name, mode);
  if (!file) {
    throw std::runtime_error("Error opening wav file");
  }
  if (std::fread(&header, 1, HEADER_SIZE, file) < HEADER_SIZE) {
    throw std::runtime_error("Error reading wav file header");
  }
}

wav_file::wav_file(const char* out_file_name, const char* mode, std::vector<wav_file> const& mono_files, double amp_multiplier): mode(mode) {
  if (mode[0] != 'w') {
    throw std::runtime_error("Can't make a file with such mode");
  }
  file = std::fopen(out_file_name, mode);
  uint32_t new_size = 0;
  uint32_t max_data_size = 0;
  for (size_t i = 0; i < mono_files.size(); i++) {
    if (mono_files[i].get_num_channels() != 1) {
      throw std::runtime_error("Can't merge stereo files");
    }
    if (i + 1 < mono_files.size() && !are_mergeable(mono_files[i], mono_files[i + 1])) {
      throw std::runtime_error("Files are not mergeable");
    }
    new_size += mono_files[i].get_data_size();
    max_data_size = std::max(max_data_size, mono_files[i].get_data_size());
  }
  memcpy(header, mono_files[0].header, HEADER_SIZE);
  set_num_channels(mono_files.size());
  set_data_size(get_num_channels() * new_size);
  set_chunk_size(20 + get_subchunk_size() + get_data_size());
  write_header();
  switch (get_bits_per_sample()) {
  case 8:
    write_merged_data<uint8_t>(mono_files, max_data_size, amp_multiplier);
    break;
  case 16:
    write_merged_data<uint16_t>(mono_files, max_data_size, amp_multiplier);
    break;
  case 32:
    write_merged_data<uint32_t>(mono_files, max_data_size, amp_multiplier);
    break;
  }
}

wav_file::~wav_file() {
  std::fclose(file);
}

uint32_t wav_file::get_data_size() const noexcept {
    return header_cast(DATA_SIZE);
}
uint16_t wav_file::get_num_channels() const noexcept {
  return header_cast(NUM_CHANNELS);
}

uint32_t wav_file::get_sample_rate() const noexcept {
  return header_cast(SAMPLE_RATE);
}

uint32_t wav_file::get_byte_rate() const noexcept {
  return header_cast(BYTE_RATE);
}

uint32_t wav_file::get_bits_per_sample() const noexcept {
  return header_cast(BITS_PER_SAMPLE);
}

u_int32_t wav_file::get_subchunk_size() const noexcept {
  return header_cast(SUBCHUNK_SIZE);
}


uint32_t wav_file::get_chunk_size() const noexcept {
  return header_cast(CHUNK_SIZE);
}

void wav_file::set_data_size(uint32_t val) noexcept {
  header_cast(DATA_SIZE) = val;
}

void wav_file::set_num_channels(uint16_t val) noexcept {
    header_cast(NUM_CHANNELS) = val;
}

void wav_file::set_sample_rate(uint32_t val) noexcept {
  header_cast(SAMPLE_RATE) = val;
}

void wav_file::set_byte_rate(uint32_t val) noexcept {
  header_cast(BYTE_RATE) = val;
}

void wav_file::set_bits_per_sample(uint32_t val) noexcept {
  header_cast(BITS_PER_SAMPLE) = val;
}

void wav_file::set_chunk_size(uint32_t val) noexcept {
  header_cast(CHUNK_SIZE) = val;
}

void wav_file::write_header() {
  std::fwrite(header, 1, HEADER_SIZE, file);
}

bool wav_file::is_writable() const noexcept {
  return mode[0] == 'w' || mode[1] == '+';
}

bool are_mergeable(wav_file const& a_file, wav_file const& b_file) {
  return a_file.get_bits_per_sample() == b_file.get_bits_per_sample() &&
         a_file.get_byte_rate() == b_file.get_byte_rate() &&
         a_file.get_sample_rate() == b_file.get_sample_rate();
}
