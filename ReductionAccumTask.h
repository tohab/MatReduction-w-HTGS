#pragma once


#include "limits.h"
#include "MatrixBlockData.h"
#include "ReductionData.h"
#include <htgs/api/ITask.hpp>

/// Accumulates ReductionDatas of blocks until all have been processed
/**
* Takes in ReductionData and accumulates it with existing values. Counts number of blocks processed; once count reaches total number of blocks in the matrix, passes on combined ReductionData.
*/

class ReductionAccumTask : public htgs::ITask<ReductionData, ReductionData> {

public:
	ReductionAccumTask(size_t numTotalBlocks) : numTotalBlocks(numTotalBlocks) {}

	virtual void executeTask(std::shared_ptr<ReductionData> data) override {
		currentSum += data->getSum();
		currentSumSq += data->getSumSq();
		if (data->getMin() < currentMin) currentMin = data->getMin();
		if (data->getMax() > currentMax) currentMax = data->getMax();

		// Increments count of blocks.
		count++;

		// Passes on ReductionData if all blocks have been counted.
		if (count >= numTotalBlocks) {
			addResult(new ReductionData(currentSum, currentSumSq, currentMin, currentMax));
		}
	}
	
	virtual std::string getName() override {
		return "ReductionAccumTask";
	}

	virtual ReductionAccumTask *copy() override {
		return new ReductionAccumTask(numTotalBlocks);
	}

private:
	double currentSum = 0;
	double currentSumSq = 0;
	double currentMin = DBL_MAX;
	double currentMax = DBL_MIN;

	size_t count = 0;
	size_t numTotalBlocks;
};