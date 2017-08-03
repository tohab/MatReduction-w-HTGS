#pragma once
#include <htgs/api/IData.hpp>
#include "MatrixType.h"
class MatrixRequestData : public htgs::IData {
public:
	MatrixRequestData(size_t row, size_t col, MatrixType type) : row(row), col(col), type(type) {}
	size_t getRow() const {
		return row;
	}
	size_t getCol() const {
		return col;
	}
	MatrixType getType() const {
		return type;
	}
private:
	size_t row;
	size_t col;
	MatrixType type;
};