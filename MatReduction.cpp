// MatrixReduction.cpp : Defines the entry point for the application.
//

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

// returns -1 if data are not the same, 0 if they are the same
// 
int validateResults(ReductionData *sequentialData, const std::shared_ptr<ReductionData>& htgsData) {
	if (sequentialData->getMax() != htgsData->getMax()) {
		std::cout << "max wrong" << std::endl;
		return -1;
	} 
	if (sequentialData->getMin() != htgsData->getMin()) {
		std::cout << "min wrong" << std::endl;
		return -1;
	} 
	if (sequentialData->getSum() != htgsData->getSum()) {
		std::cout << "sum wrong" << std::endl;
		return -1;
	} 
	if (sequentialData->getSumSq() != htgsData->getSumSq()) {
		std::cout << "Sequential sumsq: " << sequentialData->getSumSq() << ", htgs sumsq: " << htgsData->getSumSq() << std::endl;
		return -1;
	}
	std::cout << "validating" << std::endl;
	return 0;
}
ReductionData* computeSequentialReduction(double *matrix, size_t width, size_t height, size_t leadingDimension) {
	//computeMatMul(fullMatrixAHeight, fullMatrixBWidth, fullMatrixAWidth, 1.0, matrixA, fullMatrixAWidth, matrixB, fullMatrixBWidth, 0.0, matrixC, fullMatrixBWidth, false);
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
	//int numRetry = 1;
	std::ofstream runtimeFile(runtimeFileStr, std::ios::app);
	// ^ processes all arguments to command w/ MatReductionArgs

	double *matrix = new double[numElements];
	initMatrix(matrix, matrixWidth, matrixHeight, false);
	
	ReductionData *sequentialData;

	SimpleClock clk;
	SimpleClock endToEnd;

	if (runSequential) {
		// computeSequentialendToEnd.start();
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
		//std::cout << (runSequential ? "sequential" : "htgs") << ", time: " << clk.getAverageTime(TimeVal::MILLI) << "ms" << std::endl;
	}
	else {
		endToEnd.start();
		LoadMatrixTask *readMatTask = new LoadMatrixTask(matrix, numReadThreads, MatrixType::MatrixAny, blockSize, matrixWidth, matrixHeight, false);
		BlockReductionTask *blkReductionTask = new BlockReductionTask(numProdThreads);
		// passes the number of blocks to reduce to the accumulator so it can count the right number
		ReductionAccumTask *reductionAccumTask = new ReductionAccumTask(ceil((double)matrixWidth*matrixHeight / blockSize / blockSize));
		// possible source of error?

		size_t blkHeightMat = readMatTask->getNumBlocksRows();
		size_t blkWidthMat = readMatTask->getNumBlocksCols();

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

		// had not before
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
						// it appears the multiplication did not go as planned :(
						std::cout << "Validation FAILED" << std::endl;
					}
					else {
						std::cout << "Validation PASSED" << std::endl;
					}
				}
			}
		}


		runtime->waitForRuntime();
		taskGraph->writeDotToFile("output.dot");
		taskGraph->writeDotToFile("profile-graph.dot", DOTGEN_COLOR_COMP_TIME | DOTGEN_FLAG_SHOW_ALL_THREADING);


		delete runtime;
		//std::cout << (runSequential ? "sequential" : "htgs") << ", time: " << clk.getAverageTime(TimeVal::MILLI) << ", end-to-end: " << endToEnd.getAverageTime(TimeVal::MILLI) << std::endl;
	}

}