#pragma once
#include <htgs/api/MemoryData.hpp>
#include "MatrixRequestData.h"
template<class Type>
class MatrixBlockData : public htgs::IData {
public:
	MatrixBlockData(const std::shared_ptr<MatrixRequestData> &request,
		const Type &matrixData,
		size_t matrixWidth,
		size_t matrixHeight,
		size_t leadingDimension) :
		request(request), matrixData(matrixData), matrixWidth(matrixWidth), matrixHeight(matrixHeight), leadingDimension(leadingDimension) {}
	const std::shared_ptr<MatrixRequestData> &getRequest() const {
		return request;
	}
	const Type &getMatrixData() const {
		return matrixData;
	}
	size_t getMatrixWidth() const {
		return matrixWidth;
	}
	size_t getMatrixHeight() const {
		return matrixHeight;
	}
	size_t getLeadingDimension() const {
		return leadingDimension;
	}
private:
	std::shared_ptr<MatrixRequestData> request;
	Type matrixData;
	size_t matrixWidth;
	size_t matrixHeight;
	size_t leadingDimension;
};