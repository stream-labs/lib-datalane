# About Datalane
Datalane is a high performance RPC/IPC library for C/C++ (and others if wrapper code is written). It is capable of very high throughput with next to no overhead.

# Frequently Asked Questions

## Why write your own RPC/IPC library?
It simply started with the problem that current solutions are simply too slow. They are all designed for asynchronous workloads, but my workload was 99% synchronous. So it was either attempt to make the workload somehow asynchronous and deal with the latency, or find a way to make a faster synchronous RPC/IPC library.

And I chose the latter, because I wanted to explore the actual possibilities of the OS/Kernel I am working with. After all, programming is an endless task of learning new things and attempts at improving the current situation. So here we are.

## Why CMake?
CMake is one of the best tools I have worked with on Windows and Linux. It works out of the box and configuring is really easy compared to make or MSBuild. 

## Are there other Licenses available?
Yes! I offer commercial licenses for closed-source applications that are valid for a year, or endlessly if code improvements are backported into the open-source library itself. Please contact me at [info@xaymar.com](mailto://info@xaymar.com) for more info.

# License
This library is licensed under GPLv2 (or later). See the LICENSE file for more info.