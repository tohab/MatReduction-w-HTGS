#pragma once

#include "limits.h"
#include "MatrixBlockData.h"
#include "ReductionData.h"
#include <htgs/api/ITask.hpp>

/// Task to perform reduction on a single block
/**
* Takes in MatrixBlockData and reduces it to ReductionData.
*/

class BlockReductionTask : public htgs::ITask<MatrixBlockData<double *>, ReductionData> { 

public:
	BlockReductionTask(size_t numThreads) : ITask(numThreads) {}

	virtual void executeTask(std::shared_ptr<MatrixBlockData<double *>> data) override {
		double sum = 0;
		double sumSq = 0;
		double min = DBL_MAX;
		double max = DBL_MIN;

		size_t width = data->getMatrixWidth();
		size_t height = data->getMatrixHeight();

			for (size_t r = 0; r < height; r++) {
				for (size_t c = 0; c < width; c++) {
					double element = data->getMatrixData()[c + r*data->getLeadingDimension()];
					sum += element;
					sumSq += element * element;
					if (element < min)
						min = element;
					if (element > max)
						max = element;
				}
			}

		addResult(new ReductionData(sum, sumSq, min, max));
	}

	virtual std::string getName() override {
		return "BlockReductionTask";
	}

	virtual BlockReductionTask *copy() override {
		return new BlockReductionTask(this->getNumThreads());
	}

};