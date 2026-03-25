#include "markdown/file_io.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>

namespace markdown {

std::optional<std::string> ReadFileUtf8(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open()) {
    return std::nullopt;
  }

  std::ostringstream buffer;
  buffer << input.rdbuf();
  std::string content = buffer.str();

  constexpr unsigned char kBomByte0 = 0xEFU;
  constexpr unsigned char kBomByte1 = 0xBBU;
  constexpr unsigned char kBomByte2 = 0xBFU;
  if (content.size() >= 3 &&
      static_cast<unsigned char>(content.at(0)) == kBomByte0 &&
      static_cast<unsigned char>(content.at(1)) == kBomByte1 &&
      static_cast<unsigned char>(content.at(2)) == kBomByte2) {
    content = content.substr(3);
  }

  return content;
}

bool WriteFileUtf8(const std::filesystem::path& path,
                   const std::string& content, std::string* error) {
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output.is_open()) {
    if (error != nullptr) {
      *error = "Unable to open file for writing.";
    }
    return false;
  }

  output.write(content.data(), static_cast<std::streamsize>(content.size()));
  if (!output.good()) {
    if (error != nullptr) {
      *error = "Failed to write file.";
    }
    return false;
  }

  return true;
}

}  // namespace markdown
