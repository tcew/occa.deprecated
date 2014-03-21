#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

inline std::string readFile(const std::string &filename){
  std::ifstream fs(filename.c_str());
  return std::string(std::istreambuf_iterator<char>(fs),
                     std::istreambuf_iterator<char>());
}

inline std::string saveFileToVariable(std::string filename,
                                      std::string varName,
                                      int &chars,
                                      std::string indent = ""){
  std::string fileContents = readFile(filename);
  chars = fileContents.size();

  std::stringstream headerSS;

  headerSS << indent << "char " << varName << "[" << chars + 1 << "] = {";
  std::string header = headerSS.str();

  std::string tab;
  tab.assign(header.size(), ' ');

  std::stringstream ss;

  ss << header;

  ss << std::showbase
     << std::internal
     << std::setfill('0');

  for(int i = 0; i < chars; ++i){
    ss << std::hex << std::setw(4) << (int) fileContents[i] << ", ";

    if((i % 8) == 7)
      ss << '\n' << indent << tab;
  }

  ss << std::hex << std::setw(4) << 0 << "};\n";

  return ss.str();
}

int main(int argc, char **argv){
  int mpChars, clChars, cuChars;

  std::string ns = "namespace occa {";
  std::string mp = saveFileToVariable("../include/occaOpenMPDefines.hpp",
                                      "occaOpenMPDefines",
                                      mpChars,
                                      "    ");

  std::string cl = saveFileToVariable("../include/occaOpenCLDefines.hpp",
                                      "occaOpenCLDefines",
                                      clChars,
                                      "    ");

  std::string cu = saveFileToVariable("../include/occaCUDADefines.hpp",
                                      "occaCUDADefines",
                                      cuChars,
                                      "    ");

  std::ofstream fs;
  fs.open("../include/occaKernelDefines.hpp");

  fs << ns << '\n'
     << "    extern char occaOpenMPDefines[" << mpChars << "];\n"
     << "    extern char occaOpenCLDefines[" << clChars << "];\n"
     << "    extern char occaCUDADefines["   << cuChars << "];\n"
     << "}\n";

  fs.close();

  fs.open("../src/occaKernelDefines.cpp");

  fs << ns << '\n'
     << mp << '\n'
     << cl << '\n'
     << cu << '\n'
     << "}\n";

  fs.close();
}
