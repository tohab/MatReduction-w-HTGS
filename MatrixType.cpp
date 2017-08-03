#include "stdafx.h"
#include <string>
#include "MatrixType.h"

std::string matrixTypeToString(MatrixType type) {
	switch (type) {
	case MatrixType::MatrixA: return "MatrixA";
	case MatrixType::MatrixB: return "MatrixB";
	case MatrixType::MatrixC: return "MatrixC";
	case MatrixType::MatrixAny: return "MatrixAny";
	}
	return "";
}