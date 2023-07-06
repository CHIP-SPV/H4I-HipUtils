<!---
Copyright 2021-2023 UT-Battelle
See LICENSE.txt in the root of the source distribution for license info.
-->

# Overview

This library provides support needed across multiple H4I libraries - those that
allow HIP applications to target Intel GPUs - such as
[H4I-HipBLAS](https://github.com/CHIP-SPV/H4I-HipBLAS) and 
[H4I-HipSOLVER](https://github.com/CHIP-SPV/H4I-HipSOLVER).

This library relies on the MKL shim library, but is separate because the 
MKL shim library must be compiled with a SYCL-capable compiler, and this
library must be compiled with the same HIP-capable compiler as the H4I
libraries that use it.

