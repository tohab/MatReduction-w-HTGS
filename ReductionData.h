#pragma once
#include <htgs/api/IData.hpp>

class ReductionData : public htgs::IData {
	//private goes outside parens
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