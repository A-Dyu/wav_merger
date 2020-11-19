#include <iostream>
#include "wav_file.h"

int main() {
  std::vector<wav_file> mono_files;
  for (size_t i = 0; i < 4; i++) {
    std::string file_name;
    std::cin >> file_name;
    mono_files.emplace_back(file_name.data(), "r");
  }
  wav_file out("../out.wav", "w", mono_files, 2);
  return 0;
}
