#pragma once
#include <htgs/api/IData.hpp>

/// Stores data from reductions.
/**
* Contains the sum, sum of squares, minimum, and maximum of the elements of a block. Can be used to calculute average/standard deviation.
*/

class ReductionData : public htgs::IData {
public:
	ReductionData(double sum, double sumSq, double min, double max) : sum(sum), sumSq(sumSq), blockMin(min), blockMax(max) {}

	double getSum() const {
		return sum;
	}

	double getSumSq() const {
		return sumSq;
	}

	double getMin() const {
		return blockMin;
	}

	double getMax() const {
		return blockMax;
	}

private:
	double sum;
	double sumSq;
	double blockMin;
	double blockMax;
};