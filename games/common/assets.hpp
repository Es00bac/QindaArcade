// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
namespace arcade {
inline std::filesystem::path assets(){
 if(const char* p=std::getenv("QINDA_ASSETS"))return p;
 std::error_code ec;auto exe=std::filesystem::read_symlink("/proc/self/exe",ec);
 if(!ec){auto path=exe.parent_path()/"../share/qinda-arcade";if(std::filesystem::exists(path))return path;}
 return QINDA_ASSET_DIR;
}
inline std::filesystem::path config(){
 if(const char* p=std::getenv("QINDA_CONFIG"))return p;
 if(const char* p=std::getenv("XDG_CONFIG_HOME"))return std::filesystem::path(p)/"qinda-arcade";
 if(const char* p=std::getenv("HOME"))return std::filesystem::path(p)/".config/qinda-arcade";
 return std::filesystem::temp_directory_path()/"qinda-arcade";
}
}
