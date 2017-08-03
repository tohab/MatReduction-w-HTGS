// MatrixReduction.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>
#include "../../htgs-files/HTGS-tutorials/tutorial-utils/SimpleClock.h"
#include "../../htgs-files/HTGS-tutorials/tutorial-utils/util-matrix.h"
#include "../../htgs-files/HTGS-tutorials/tutorial-utils/util-matrix.cpp"
#include "../../htgs-files/HTGS-tutorials/tutorial-utils/util-filesystem.h"
#include "../../htgs-files/HTGS-tutorials/tutorial-utils/util-filesystem.cpp"
#include "MatrixBlockData.h"
#include "MatrixRequestData.h"
#include "ReductionData.h"
#include "BlockReductionTask.h"
#include "LoadMatrixTask.h"
#include "ReductionAccumTask.h"
#include "MatReductionArgs.h"

int validateResults(double *matrixC, double *matrixC_HTGS, size_t fullMatrixAHeight, size_t fullMatrixBWidth) {
	/*if (!validateMatMulResults(20, matrixC, matrixC_HTGS, fullMatrixAHeight*fullMatrixBWidth)) {
	return -1;
	}*/
	return 0;
}
void computeSequentialReduction(double *matrix, size_t fullMatrixHeight, size_t fullMatrixWidth) {
	//computeMatMul(fullMatrixAHeight, fullMatrixBWidth, fullMatrixAWidth, 1.0, matrixA, fullMatrixAWidth, matrixB, fullMatrixBWidth, 0.0, matrixC, fullMatrixBWidth, false);
}

int main(int argc, char *argv[]) {
	std::cout << "hiii this should be printing" << std::endl;

	MatReductionArgs matReductionArgs;
	matReductionArgs.processArgs(argc, argv);
	size_t matrixHeight = matReductionArgs.getMatrixHeight();
	size_t matrixWidth = matReductionArgs.getMatrixWidth();
	size_t blockSize = matReductionArgs.getBlockSize();
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

	double *matrix = new double[matrixHeight * matrixWidth];
	initMatrix(matrix, matrixWidth, matrixHeight, false);

	SimpleClock clk;
	SimpleClock endToEnd;

	if (runSequential) {
		// computeSequentialMatMu);
		std::cout << "sequential reduction not implemented yet" << std::endl;
	}
	else {
		endToEnd.start();
		LoadMatrixTask *readMatTask = new LoadMatrixTask(matrix, numReadThreads, MatrixType::MatrixAny, blockSize, matrixWidth, matrixHeight, false);
		BlockReductionTask *blkReductionTask = new BlockReductionTask(numProdThreads);
		// passes the number of blocks to reduce to the accumulator so it can count the right number
		ReductionAccumTask *reductionAccumTask = new ReductionAccumTask(ceil((double)matrixWidth*matrixHeight / blockSize));
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

		while (!taskGraph->isOutputTerminated()) {
			auto data = taskGraph->consumeData();
			if (data != nullptr) {
				std::cout << "total sum is..." << data->getSum() << std::endl;
			}
		}
		runtime->waitForRuntime();
		clk.stopAndIncrement();
		delete runtime;
		endToEnd.stopAndIncrement();
	}

}