#include "wav_file.h"
wav_file::wav_file() noexcept : file(nullptr), name(nullptr), mode(nullptr) {}

wav_file::wav_file(const char* file_name, const char* mode) : wav_file() {
  load(file_name, mode);
}

wav_file::wav_file(wav_file&& other) noexcept : wav_file_header(std::move(other)), file(other.file), name(other.name), mode(other.mode) {
  other.mode = nullptr;
  other.file = nullptr;
  other.name = other.mode = nullptr;
}

wav_file &wav_file::operator=(wav_file&& other) noexcept {
  if (this != &other) {
    free();
    new(this) wav_file(std::move(other));
  }
  return *this;
}

wav_file::~wav_file() {
  free();
}

void wav_file::load(const char* file_name, const char* file_mode) {
  if (file_mode[0] != 'r' && file_mode[1] != '+') {
    throw std::runtime_error("Can't load file without reading");
  }
  free();
  name = file_name;
  mode = file_mode;
  file = std::fopen(file_name, mode);
  if (!file) {
    throw std::runtime_error("Error opening wav file");
  }
  check_chunk_id("RIFF");
  auto chunk_size = read_number<uint32_t>();
  check_chunk_id("WAVE");
  check_chunk_id("fmt ");
  auto fmt_size = read_number<uint32_t>();
  if (read_number<uint16_t>() != 1) {
    throw std::runtime_error("non-PCM files are not supported");
  }
  n_channels = read_number<uint16_t>();
  sample_rate = read_number<uint32_t>();
  byte_rate = read_number<uint32_t>();
  block_align = read_number<uint16_t>();
  bits_per_sample = read_number<uint16_t>();
  std::fseek(file, fmt_size - 16, SEEK_CUR);
  check_chunk_id("data");
  data_size = read_number<uint32_t>();
  data_begin = chunk_size - data_size + 8;
}

void wav_file::save(const char* new_file_name) {
  FILE* old_file = file;
  const char* old_name = name;
  std::fseek(old_file, data_begin, SEEK_SET);
  name = new_file_name;
  mode = "w+";
  file = std::fopen(name, mode);
  write_header();
  for (uint32_t i = 0; i < data_size; i++) {
    char b;
    std::fread(&b, 1, 1, old_file);
    std::fwrite(&b, 1, 1, file);
  }
  std::fclose(old_file);
  std::remove(old_name);
}

void wav_file::close() {
  free();
  name = mode = nullptr;
  file = nullptr;
}

uint32_t wav_file::get_data_size() const noexcept {
  return data_size;
}
uint16_t wav_file::get_num_channels() const noexcept {
  return n_channels;
}

uint32_t wav_file::get_sample_rate() const noexcept {
  return sample_rate;
}

uint32_t wav_file::get_byte_rate() const noexcept {
  return byte_rate;
}

uint32_t wav_file::get_bits_per_sample() const noexcept {
  return bits_per_sample;
}

void wav_file::write_header() {
  write_chunk_id("RIFF");
  write_number(data_size + 28);
  write_chunk_id("WAVE");
  write_chunk_id("fmt ");
  write_number<uint32_t>(16);
  write_number<uint16_t>(1);
  write_number(n_channels);
  write_number(sample_rate);
  write_number(byte_rate);
  write_number(block_align);
  write_number(bits_per_sample);
  write_chunk_id("data");
  write_number(data_size);
  data_begin = 36;
}

bool wav_file::is_writable() const noexcept {
  return mode[0] == 'w' || mode[1] == '+';
}

std::string wav_file::read_chunk_header() const {
  char id[4];
  read_n_bytes(id, 4);
  return std::string(id, 4);
}
void wav_file::read_n_bytes(char *dst, size_t n) const {
  if (std::fread(dst, 1, n, file) < 1) {
    throw std::runtime_error("Error reading wav file");
  }
}
void wav_file::check_chunk_id(std::string const& id) const {
  if (read_chunk_header() != id) {
    throw std::runtime_error("Invalid wav file");
  }
}
void wav_file::write_chunk_id(const char* id) {
  std::fwrite(id, 1, 4, file);
}
void wav_file::free() {
  if (file) {
    std::fclose(file);
  }
}

wav_file merge(const char* out_file_name, std::vector<wav_file> const& mono_files, double amp_multiplier) {
  wav_file out_file;
  assert(!mono_files.empty());
  out_file.file = std::fopen(out_file_name, "w+");
  if (!out_file.file) {
    throw std::runtime_error("Error while creating output file");
  }
  out_file.name = out_file_name;
  out_file.mode = "w+";
  uint32_t max_data_size = 0;
  for (size_t i = 0; i < mono_files.size(); i++) {
    std::fseek(mono_files[i].file, mono_files[i].data_begin, SEEK_SET);
    if (mono_files[i].get_num_channels() != 1) {
      throw std::runtime_error("Can't merge stereo files");
    }
    if (i + 1 < mono_files.size() && !are_mergeable(mono_files[i], mono_files[i + 1])) {
      throw std::runtime_error("Files are not mergeable");
    }
    max_data_size = std::max(max_data_size, mono_files[i].get_data_size());
  }
  *(reinterpret_cast<wav_file_header*>(&out_file)) = *(reinterpret_cast<wav_file_header const*>(&mono_files[0]));
  out_file.n_channels = 2;
  out_file.data_size = 2 * max_data_size;
  out_file.write_header();
  switch (out_file.bits_per_sample) {
  case 8:
    out_file.write_merged_data<uint8_t>(mono_files, max_data_size, amp_multiplier);
    break;
  case 16:
    out_file.write_merged_data<int16_t>(mono_files, max_data_size, amp_multiplier);
    break;
  case 32:
    out_file.write_merged_data<int32_t>(mono_files, max_data_size, amp_multiplier);
    break;
  }
  return out_file;
}

bool are_mergeable(wav_file const& a_file, wav_file const& b_file) {
  return a_file.get_bits_per_sample() == b_file.get_bits_per_sample() &&
         a_file.get_byte_rate() == b_file.get_byte_rate() &&
         a_file.get_sample_rate() == b_file.get_sample_rate();
}
