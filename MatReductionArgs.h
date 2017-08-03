//
// Created by tjb3 on 3/21/17.
//

#ifndef HTGS_TUTORIALS_MATMULARGS_H
#define HTGS_TUTORIALS_MATMULARGS_H

#include <cstddef>
#include <string>
#include <iostream>
#include <sstream>

class MatReductionArgs {
 public:

  MatReductionArgs() {
    matrixHeight = 1024;
    matrixWidth = 1024;

    blockSize = 256;
    numReadThreads = 1;
    numReductionThreads = 4;
    directory = "data";
    outputDir = directory;
    runSequential = false;
    validateResults = false;
  }

  size_t getMatrixHeight() const {
    return matrixHeight;
  }
  size_t getMatrixWidth() const {
    return matrixWidth;
  }
  size_t getBlockSize() const {
    return blockSize;
  }
  size_t getNumReadThreads() const {
    return numReadThreads;
  }
  size_t getNumReductionThreads() const {
    return numReductionThreads;
  }
  const std::string &getDirectory() const {
    return directory;
  }
  const std::string &getOutputDir() const {
    return outputDir;
  }
  bool isRunSequential() const {
    return runSequential;
  }
  bool isValidateResults() const {
    return validateResults;
  }

  void processArgs(int argc, char** argv);

 private:
  size_t matrixHeight;
  size_t matrixWidth;
  size_t blockSize;
  size_t numReadThreads;
  size_t numReductionThreads;
  std::string directory;
  std::string outputDir;

  bool runSequential;
  bool validateResults;
};

#endif //HTGS_TUTORIALS_MATMULARGS_H
