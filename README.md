# Sparse-Graph-Based-Codes-for-Sub-Nyquist-Sampling
FFAST: https://www.eecs.berkeley.edu/~kannanr/project_ffft.html
previous name Pulse see Phd Thesis EECS-2013-215.pdf


It otstanding method related to intersection of Coding Theory, Sampling Theory und Comressed Sensing under Expander ( RIP-2).
For detail see lecture from NASIT 2019 Padovani Lecture - Kannan Ramchandran - On duality, encryption, sampling and learning https://www.youtube.com/watch?v=OeVImz3f2MI, it extended and detailed version of lecture from ISIT 2018 - Kannan Ramchandran - CODES for SPEED: the CLEAR advantage https://www.youtube.com/watch?v=HkUFn01hEPA

Project from https://github.com/UCBASiCS/FFAST build under x64 Microsoft Visual Studio 2019.

Fast Fourier Aliasing-based Sparse Transform (FFAST) for calculating the Discrete Time Fourier (DFT) transform signals having sparse spectrum.

The distribution contains C++ source code for the FFAST engine, demo files and MATLAB scripts that interact with the tool.

Idea behind FFAST
The algorithm cleverly induces sparse graph alias codes in the DFT domain, by sub-sampling of the time-domain samples. The resulting sparse graph alias codes are then exploited in a peeling decoder.

Besides the noiseless and exactly sparse spectrum, the algorithm can handle the noisy setting. The algorithm is also observed to perform well in the case of off-grid spectra; however, theoretical guarantees are not given for that case.

Example usage
Display help

  ./ffast --help
Run an FFAST experiment (-a) on randomly generated signals of length 124950 (-n 124950) having 10 sparse spectrum (-k 10) 30 times (-i 30)

  ./ffast -a -n 124950 -k 10 -i 30
Run FFAST on input data saved in inFile.txt (-f inFile.txt)

  ./ffast -f inFile.txt
  
  
  
  
References
Computing a k-sparse n-length Discrete Fourier Transform using at most 4k samples and O(k logk) complexity -- S. Pawar and K. Ramchandran
A robust FFAST framework for computing a k-sparse n-length DFT in O(k log n) sample complexity using sparse-graph codes -- S. Pawar and K. Ramchandran
Robustifying the Sparse Walsh-Hadamard Transform without Increasing the Sample Complexity of O(K log N) -- X. Li, J.K. Bradley, S. Pawar, K. Ramchandran
Authors
Quentin Byron, Thibault Derousseaux, Rohan Varma, Xiao (Simon) Li, Orhan Ocal

