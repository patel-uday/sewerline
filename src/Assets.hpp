#pragma once
#include <cstddef>
#include <string_view>

struct AssetData {
  const void* data{};
  std::size_t size{};
  explicit operator bool() const { return data && size; }
};

// Decode embedded lossless resources into memory. No temp extraction.
AssetData embeddedAsset(std::string_view path);
