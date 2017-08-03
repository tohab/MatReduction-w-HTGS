Matrix Reduction with the Hybrid Task-Graph Scheduler (HTGS)
08/03/2017
by Rohan Prasad and Tim Blattner, National Institute of Standards and Technology

This program uses the HTGS framework to implement matrix reduction, that is, reducing a large matrix to its standard deviation, average, minimum and maximum. Tested on Windows 10 Enterprise with Microsoft Visual Studio Community (2017).  

General data-flow:
(MatrixRequestData)-->LoadMatrixTask--(MatrixBlockData)-->BlockReductionTask--(ReductionData)-->ReductionAccumTask--(ReductionData)-->ReductionAccumTask(loops until complete)-->(ReductionData) 

Specifics are best seen in the mainfile, MatReduction.cpp. Because of the specified pipeline and the HTGS API, the program is parrallelized and can be several times faster than a sequential implementation. 

Installation instructions for HTGS and more details can be found here:
https://github.com/usnistgov/HTGS-Tutorials
