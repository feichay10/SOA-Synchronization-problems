# SOA Synchronization problems: Temperature

Made by Cheuk Kelly Ng Pante

# Description
In this project I have programmed a CPU temperature statistics calculator for the whole year each day. I have worked in QTCreator on 5.15.2v.

I have used QCoreApplication, QVector, Qmutex, QWaitCondition, QWriteLocker to make the project.

# Code
I have used a template that they have given us to make a set of modifications. I have 4 codes to divide the code:
* `mainSerial.cpp`: It is where the serial mode is.
* `mainProducer-Consumer.cpp`: It is where the Producer/Consumer mode is.
* `mainReaderWriter.cpp`: It is where the Reader/Writer mode is.
* `main.cpp`: It is where the three modes are.

The objective of the project is to check which of these synchronization problems is the most effective at runtime. I have a 30 days buffer capacity. It's a cyclic buffer. I have to calculate the mean and the median of every day on a year.

# Serial mode
The serial mode is a for loop that starts at 0 and ends at *total* in which "every seconds" I put a random value, between 50 and 100 into the buffer.

I have an *if* that searches if the iterator is in another day to calculate the mean and the median of a previous day.

# Producer/Consumer mode
The Producer/Consumer mode has three threads. One of these is the producer and the rest are consumers.

In this problem, the producer is too fast, therefore the producer is waiting most of the time for consumers because the buffer is full.

* Producer: It has `mutex.lock()` in two occasions: To check if the buffer is full or to check if the amount he has produced is more than 2 days already. I check if it produce 2 days **to wake** both threads at the same time. Producer also increments a global variable named comparator, this variable is used to know if the buffer is full or "empty". 

* Consumer: It has `mutex.lock()` in two occasions: To check if the buffer is "empty" or to decrement the comparator one day in seconds and wake the producer. The consumer increases day by day because the producer is too fast.

# Readers/Writer mode
The Readers/Writer mode has three threads. One of these is the writer and the rest are readers.

As the Producer-Consumer mode, the writer is also too fast for the readers.

Writer and readers work the same way as producer/consumer problem but readers work with "shared lock" LockForRead, and the writer works with "unique lock" LockForWrite.

# Comparative tables
|         | Serial Mode  | Producer/Consumer mode | Readers/Writer mode   |
| :-----: | :----------: | :--------------------: | :-------------------: |
| seconds | 8.85346      | 35.5727                | 141.744               |
