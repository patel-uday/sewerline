param([Parameter(Mandatory=$true)][string]$OutputDir)
$ErrorActionPreference = "Stop"
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class AssetCompressor {
 [DllImport("cabinet.dll", SetLastError=true)]
 static extern bool CreateCompressor(uint algorithm, IntPtr allocation, out IntPtr compressor);
 [DllImport("cabinet.dll", SetLastError=true)]
 static extern bool Compress(IntPtr compressor, byte[] source, UIntPtr sourceSize, byte[] destination, UIntPtr capacity, out UIntPtr written);
 [DllImport("cabinet.dll")] static extern bool CloseCompressor(IntPtr compressor);
 [DllImport("cabinet.dll")] static extern bool CreateDecompressor(uint algorithm, IntPtr allocation, out IntPtr decompressor);
 [DllImport("cabinet.dll")] static extern bool Decompress(IntPtr decompressor, byte[] source, UIntPtr sourceSize, byte[] destination, UIntPtr capacity, out UIntPtr written);
 [DllImport("cabinet.dll")] static extern bool CloseDecompressor(IntPtr decompressor);
 public static byte[] Pack(byte[] source) {
  IntPtr handle;
  if(!CreateCompressor(5,IntPtr.Zero,out handle)) throw new Exception("Cannot create LZMS compressor");
  try {
   UIntPtr size;
   Compress(handle,source,(UIntPtr)source.Length,null,UIntPtr.Zero,out size);
   var packed=new byte[checked((int)size.ToUInt64())];
   if(!Compress(handle,source,(UIntPtr)source.Length,packed,(UIntPtr)packed.Length,out size))
    throw new Exception("Asset compression failed");
   Array.Resize(ref packed,checked((int)size.ToUInt64()));
   IntPtr decoder;
   if(!CreateDecompressor(5,IntPtr.Zero,out decoder)) throw new Exception("Cannot verify packed asset");
   try {
    var restored=new byte[source.Length];
    if(!Decompress(decoder,packed,(UIntPtr)packed.Length,restored,(UIntPtr)restored.Length,out size) || size.ToUInt64()!=(ulong)source.Length)
     throw new Exception("Asset round trip failed");
    for(int i=0;i<source.Length;i++) if(source[i]!=restored[i]) throw new Exception("Asset bytes changed");
   } finally { CloseDecompressor(decoder); }
   return packed;
  } finally { CloseCompressor(handle); }
 }
}
'@
$root=Split-Path $PSScriptRoot
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
foreach($file in Get-ChildItem (Join-Path $root "assets") -Recurse -File | Where-Object Extension -in '.png','.wav'){
 $name=$file.Name+'.bin'
 $packed=[AssetCompressor]::Pack([IO.File]::ReadAllBytes($file.FullName))
 [IO.File]::WriteAllBytes((Join-Path $OutputDir $name),$packed)
}
