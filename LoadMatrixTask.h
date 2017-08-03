#pragma once
#include <htgs/api/ITask.hpp>
#include <cmath>
#include "MatrixBlockData.h"
#include "MatrixRequestData.h"
#include "../../HTGS-Tutorials/tutorial-utils/util-matrix.h"

// Task to load a pointer to a block in the matrix
/**
* Takes in MatrixRequestData, which specifies which row/col to row, and outputs matrix block data at the location.
*/

class LoadMatrixTask : public htgs::ITask<MatrixRequestData, MatrixBlockData<double *>> {
public:
	LoadMatrixTask(double *matrix, size_t numThreads, MatrixType matrixType, size_t blockSize, size_t fullMatrixWidth, size_t fullMatrixHeight, bool colMajor) :
		ITask(numThreads),
		matrix(matrix),
		blockSize(blockSize),
		fullMatrixHeight(fullMatrixHeight),
		fullMatrixWidth(fullMatrixWidth),
		matrixType(matrixType),
		colMajor(colMajor)
	{
		numBlocksRows = (size_t)ceil((double)fullMatrixHeight / (double)blockSize);
		numBlocksCols = (size_t)ceil((double)fullMatrixWidth / (double)blockSize);
	}

	virtual ~LoadMatrixTask() {}

	virtual void executeTask(std::shared_ptr<MatrixRequestData> data) {
		size_t row = data->getRow();
		size_t col = data->getCol();
		size_t matrixWidth;
		size_t matrixHeight;
		if (col == numBlocksCols - 1 && fullMatrixWidth % blockSize != 0)
			matrixWidth = fullMatrixWidth % blockSize;
		else
			matrixWidth = blockSize;
		if (row == numBlocksRows - 1 && fullMatrixHeight % blockSize != 0)
			matrixHeight = fullMatrixHeight % blockSize;
		else
			matrixHeight = blockSize;
		double *memPtr;

		// Computes starting location of pointer by converting 2D description of block to 1D matrix in memory.
		if (colMajor)
			memPtr = &matrix[IDX2C(blockSize*row, blockSize*col, fullMatrixHeight)];
		else
			memPtr = &matrix[blockSize * col + blockSize * row * fullMatrixWidth];

		if (colMajor)
			addResult(new MatrixBlockData<double *>(data, memPtr, matrixWidth, matrixHeight, fullMatrixHeight));
		else
			addResult(new MatrixBlockData<double *>(data, memPtr, matrixWidth, matrixHeight, fullMatrixWidth));
	}
	virtual std::string getName() {
		return "LoadMatrixTask(" + matrixTypeToString(matrixType) + ")";
	}
	virtual LoadMatrixTask *copy() {
		return new LoadMatrixTask(matrix, this->getNumThreads(), matrixType, blockSize, fullMatrixWidth, fullMatrixHeight, colMajor);
	}
	size_t getNumBlocksRows() const {
		return numBlocksRows;
	}
	size_t getNumBlocksCols() const {
		return numBlocksCols;
	}
private:
	double *matrix;
	size_t blockSize;
	size_t fullMatrixWidth;
	size_t fullMatrixHeight;
	size_t numBlocksRows;
	size_t numBlocksCols;
	MatrixType matrixType;
	bool colMajor;
};