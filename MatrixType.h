#pragma once
#ifndef HTGS_TUTORIALS_MATRIXTYPE_H
#define HTGS_TUTORIALS_MATRIXTYPE_H
#include <string>
enum class MatrixType {
	MatrixA,
	MatrixB,
	MatrixC,
	MatrixAny
};

std::string matrixTypeToString(MatrixType type);

#endif //HTGS_TUTORIALS_MATRIXTYPE_H