// MatrixReduction.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "resource.h"
#include <math.h>

#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>
#include "../../HTGS-tutorials/tutorial-utils/SimpleClock.h"
#include "../../HTGS-tutorials/tutorial-utils/util-matrix.h"
#include "../../HTGS-tutorials/tutorial-utils/util-matrix.cpp"
#include "../../HTGS-tutorials/tutorial-utils/util-filesystem.h"
#include "../../HTGS-tutorials/tutorial-utils/util-filesystem.cpp"
#include "MatrixBlockData.h"
#include "MatrixRequestData.h"
#include "ReductionData.h"
#include "BlockReductionTask.h"
#include "LoadMatrixTask.h"
#include "ReductionAccumTask.h"
#include "MatReductionArgs.h"

/// Mainfile for matrix reduction.

// Returns -1 if reduction data does not match, and prints which part is incorrect. Returns 0 if matches.
// 
int validateResults(ReductionData *sequentialData, const std::shared_ptr<ReductionData>& htgsData) {
	if (sequentialData->getMax() != htgsData->getMax()) {
		std::cout << "Max value is different" << std::endl;
		return -1;
	} 
	if (sequentialData->getMin() != htgsData->getMin()) {
		std::cout << "Min value is different" << std::endl;
		return -1;
	} 
	if (sequentialData->getSum() != htgsData->getSum()) {
		std::cout << "Sum is different" << std::endl;
		return -1;
	} 
	if (sequentialData->getSumSq() != htgsData->getSumSq()) {
		std::cout << "Sum of squares is different" << std::endl;
		return -1;
	}
	std::cout << "validating" << std::endl;
	return 0;
}

// Reduces the matrix to ReductionData without HTGS.
ReductionData* computeSequentialReduction(double *matrix, size_t width, size_t height, size_t leadingDimension) {
	double sum = 0;
	double sumSq = 0;
	double min = DBL_MAX;
	double max = DBL_MIN;

	for (size_t r = 0; r < height; r++) {
		for (size_t c = 0; c < width; c++) {
			double element = matrix[c + r*leadingDimension];
			sum += element;
			sumSq += element * element;
			if (element < min)
				min = element;
			if (element > max)
				max = element;
		}
	}
	return new ReductionData(sum, sumSq, min, max);
}

int main(int argc, char *argv[]) {

	// Processses all command-line arguments to variables.
	MatReductionArgs matReductionArgs;
	matReductionArgs.processArgs(argc, argv);
	size_t matrixHeight = matReductionArgs.getMatrixHeight();
	size_t matrixWidth = matReductionArgs.getMatrixWidth();
	size_t numElements = matrixWidth*matrixHeight;
	size_t blockSize = matReductionArgs.getBlockSize();
	if (blockSize > matrixWidth) {
		blockSize = matrixWidth;
		std::cout << "Warning: blockSize too large, setting to smallest dimension" << std::endl;
	} 
	if (blockSize > matrixHeight) {
		blockSize = matrixHeight;
		std::cout << "Warning: blockSize too large, setting to smallest dimension" << std::endl;
	}
	size_t numReadThreads = matReductionArgs.getNumReadThreads();
	size_t numProdThreads = matReductionArgs.getNumReductionThreads();
	std::string directory = matReductionArgs.getDirectory();
	std::string outputDirectory = matReductionArgs.getOutputDir();
	bool runSequential = matReductionArgs.isRunSequential();
	bool validate = matReductionArgs.isValidateResults();
	std::string runtimeFileStr("runtimes");

	std::ofstream runtimeFile(runtimeFileStr, std::ios::app);

	double *matrix = new double[numElements];
	// initMatrix generates the values of the matrix, and is defined in utilmatrix.cpp in the HTGS-tutorials library
	initMatrix(matrix, matrixWidth, matrixHeight, false);
	
	ReductionData *sequentialData;

	// Starts a clock to get the program runtime.
	SimpleClock clk;
	SimpleClock endToEnd;

	if (runSequential) {
		clk.start();
		endToEnd.start();

		sequentialData = computeSequentialReduction(matrix, matrixWidth, matrixHeight, matrixWidth);

		clk.stopAndIncrement();

		double totalSumSq = sequentialData->getSumSq();
		double totalSum = sequentialData->getSum();
		double average = totalSum / numElements;
		double variance = (totalSumSq - totalSum*totalSum / numElements) / numElements; // SD^2 = (sumSq - sum^2 / N) / N

		endToEnd.stopAndIncrement();

		std::cout << std::fixed << "Sequential, " << "average: " << average << ", standard deviation: " << sqrt(variance) << ", minimum: " << sequentialData->getMin() << ", maximum: " << sequentialData->getMax() << std::endl;
		std::cout <<  "Time: " << clk.getAverageTime(TimeVal::MILLI) << ", " << endToEnd.getAverageTime(TimeVal::MILLI) << std::endl; 
	}
	else {
		endToEnd.start();

		// Initiate all tasks.
		LoadMatrixTask *readMatTask = new LoadMatrixTask(matrix, numReadThreads, MatrixType::MatrixAny, blockSize, matrixWidth, matrixHeight, false);
		BlockReductionTask *blkReductionTask = new BlockReductionTask(numProdThreads);
		// The number of total blocks must be passed to the accumulator
		ReductionAccumTask *reductionAccumTask = new ReductionAccumTask(ceil((double)matrixWidth*matrixHeight / blockSize / blockSize));

		size_t blkHeightMat = readMatTask->getNumBlocksRows();
		size_t blkWidthMat = readMatTask->getNumBlocksCols();

		// Makes Taskgraph and add edges and tasks
		auto taskGraph = new htgs::TaskGraphConf<MatrixRequestData, ReductionData>();

		taskGraph->setGraphConsumerTask(readMatTask);
		taskGraph->addEdge(readMatTask, blkReductionTask);
		taskGraph->addEdge(blkReductionTask, reductionAccumTask);
		taskGraph->addGraphProducerTask(reductionAccumTask);

		htgs::TaskGraphRuntime *runtime = new htgs::TaskGraphRuntime(taskGraph);
		clk.start();
		runtime->executeRuntime();

		for (size_t col = 0; col < blkWidthMat; col++) {
			for (size_t row = 0; row < blkHeightMat; row++) {
				MatrixRequestData *mat = new MatrixRequestData(row, col, MatrixType::MatrixAny);
				taskGraph->produceData(mat);
			}
		}
		taskGraph->finishedProducingData();

		while (!taskGraph->isOutputTerminated()) {
			auto data = taskGraph->consumeData();
			clk.stopAndIncrement();
			if (data != nullptr) {
				double totalSumSq = data->getSumSq();
				double totalSum = data->getSum();
				double average = totalSum / numElements;
				// SD^2 = (sumSq - sum^2 / N) / N
				double variance = (totalSumSq - totalSum*totalSum / numElements) / numElements;

				endToEnd.stopAndIncrement();

				std::cout << std::fixed << "HTGS, " << "average: " << average << ", standard deviation: " << sqrt(variance) << ", minimum: " << data->getMin() << ", maximum: " << data->getMax() << std::endl;
				std::cout <<  "Time: " << clk.getAverageTime(TimeVal::MILLI) << ", " << endToEnd.getAverageTime(TimeVal::MILLI) << std::endl; 

				if (validate) {
					sequentialData = computeSequentialReduction(matrix, matrixWidth, matrixHeight, matrixWidth);
					int res = validateResults(sequentialData, data);
					if (res != 0) {
						std::cout << "Validation FAILED" << std::endl;
					}
					else {
						std::cout << "Validation PASSED" << std::endl;
					}
				}
			}
		}

		runtime->waitForRuntime();
		// taskGraph->writeDotToFile("output.dot"); // plain output, if desired
		taskGraph->writeDotToFile("profile-graph.dot", DOTGEN_COLOR_COMP_TIME | DOTGEN_FLAG_SHOW_ALL_THREADING); // Needs profiling enabled for full effect

		delete runtime;
	}

}