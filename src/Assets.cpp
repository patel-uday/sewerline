#include "Assets.hpp"
#include "AssetIndex.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <compressapi.h>
#include <map>
#include <vector>

AssetData embeddedAsset(std::string_view path) {
  const int id = assetId(path);
  if (!id) return {};
  // Decode each asset once, retain its bytes for the lifetime of the process.
  static std::map<int, std::vector<unsigned char>> cache;
  if (const auto found = cache.find(id); found != cache.end())
    return {found->second.data(), found->second.size()};
  const HMODULE module = GetModuleHandleW(nullptr);
  const HRSRC resource = FindResourceW(module, MAKEINTRESOURCEW(id), MAKEINTRESOURCEW(10));
  if (!resource) return {};
  const HGLOBAL loaded = LoadResource(module, resource);
  if (!loaded) return {};
  DECOMPRESSOR_HANDLE decoder{};
  if (!CreateDecompressor(COMPRESS_ALGORITHM_LZMS, nullptr, &decoder)) return {};
  const void* packed = LockResource(loaded);
  const SIZE_T packedSize = SizeofResource(module, resource);
  SIZE_T size{};
  Decompress(decoder, packed, packedSize, nullptr, 0, &size);
  if (!size) { CloseDecompressor(decoder); return {}; }
  std::vector<unsigned char> bytes(size);
  const bool ok = Decompress(decoder, packed, packedSize, bytes.data(), bytes.size(), &size);
  CloseDecompressor(decoder);
  if (!ok) return {};
  auto& saved = cache.emplace(id, std::move(bytes)).first->second;
  return {saved.data(), saved.size()};
}
