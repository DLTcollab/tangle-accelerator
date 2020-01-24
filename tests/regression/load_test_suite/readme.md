# Load test 
## What is Load test?
This is an [introduction](https://www.blazemeter.com/blog/performance-testing-vs-load-testing-vs-stress-testing/?utm_source=blog&utm_medium=BM_blog&utm_campaign=why-load-testing-is-important) of load test from a tool `BlazeMeter`

Extract from it:

> Load testing is testing that checks how systems function under a heavy number of concurrent virtual users performing transactions over a certain period of time. Or in other words, how systems handle heavy load volumes. 

> Stress testing is testing that checks the upper limits of your system by testing it under `extreme loads.`

This is the reason why this `load test` package can implement `stress test` when giving an enormous number of client.

## Implementation of Send Transfer Load test
Load test is based on common package build for unit test.

The most important part of package is the following function `SendTransfer().test()`
I use `multiprocessing` in python to create virtual clients.
With mulitprocessing.Manager() package, it'll collect `error_rate` and `response time` of each client.
Every client will send transfers depends on given number `transfer_num` for every sample.

`SendTransfer().test()` will collect samples from `client_min` to `client_max` moved with `stepsize`.
Ascending client number is benefit to observe how memory usage increase.

The elapsed time in load test is a crucial statistics, which is response time here.
We can use dumpfile() to dump average response time, standard deviation of response time, and error rate.
The plot() function give us a brief view of how response time scattered.

