# covert_channel_chatbot

## 1. Usage

This chatbot is extremely easy to use. After 'make', you can run two clients (by running ./client) to build this 
covert channel. You will need to turn one client into a RECEIVER by typing 'recv' and pressing enter. Type some 
characters on the SENDER side and press enter. The message will be magically transmitted to the RECEIVER as you will see. 

The SENDER will also show the bandwidth(B/s) of transmission.

## 2. Extra missions finished

We finished:
1) TRX. -- 10pts
2) SpeedRun. -- 10pts (we were able to improve the bandwidth as more than 10x the TA solution)
3) AnyCore. -- 30pts (no need to use taskset)
4) AnyCore++.. -- 20pts (we did not use RDRAND/RDSEED)

## 3. Challenges

In this lab assignment, our first attemp was using prime+probe alike method to attack L1 cache. However, we could not 
achieve what we expected, and the cache line hit measured by RECEIVER did not match the cache line probe by SENDER. 
We speculated that this is caused by the hardware prefetcher between L1/L2. Also, there were significant amount of noise 
when targeting L1 cache. Therefore we turned on L3 cache and used flush+reload method to attack it. The result was amazing.

