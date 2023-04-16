#include <math.h>
#include <QCoreApplication>
#include <QVector>
#include <iostream>
#include <thread>
#include <QMutex>
#include <QWaitCondition>
#include <unistd.h>
#include <condition_variable>
#include <QReadWriteLock>

QMutex mutex;
const unsigned totalBufferSize = 60 * 60 * 24 * 30;
const unsigned total = 60 * 60 * 24 * 365;  // sec * min * hour * days
const unsigned dataDay = 60 * 60 * 24;
QWaitCondition buffertNotEmpty;
QWaitCondition bufferNotFull;
QVector<float> Buffer;
unsigned comparator = 0;

double getMeanConsumer(QVector<float>* data, unsigned startPos) {
  double sum = 0.0;
  for (int i = startPos; i < startPos + dataDay; i++) {
      sum += (*data)[i % totalBufferSize];
  }
  return sum / dataDay;
}

double getMedianConsumer(QVector<float>* data, unsigned startPos) {
    std::sort((*data).begin() + startPos, (*data).begin() + startPos + dataDay -1);
    if ((startPos + dataDay - 1) % 2 == 0) {
        return (*data)[((startPos + dataDay - 1) / 2) - 1] + (*data)[(startPos + dataDay - 1) / 2] / 2.0;
    }
    return (*data)[(startPos + dataDay - 1) / 2];
}

QReadWriteLock lock;

void writer()
{
    for (unsigned int i = 0; i < total; i++) {
        lock.lockForWrite();
        if (comparator == totalBufferSize) {
            bufferNotFull.wait(&lock);
        }
        lock.unlock();

        Buffer[i % totalBufferSize] = random() % 50 + 50;
        comparator++;

        lock.lockForWrite();
        if (comparator >= dataDay * 2) {
            buffertNotEmpty.wakeAll();
        }
        lock.unlock();
    }
}

void reader(unsigned int startPos)
{
    unsigned int count = startPos;
    while (count < total) {
        lock.lockForRead();
        if (comparator < dataDay) {
            buffertNotEmpty.wait(&lock);
        }
        lock.unlock();

        std::cout << count << " of " << total << " ";
        std::cout << "average temperature " << getMeanConsumer(&Buffer, count % totalBufferSize) << " ";
        std::cout << " with median " << getMedianConsumer(&Buffer, count % totalBufferSize) << "\n";

        lock.lockForRead();
        comparator = comparator - dataDay;
        bufferNotFull.wakeAll();
        lock.unlock();

        count += dataDay * 2;
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    Buffer.resize(total);

    auto startWriterReader = std::chrono::high_resolution_clock::now();
    std::thread w(writer), r1(reader, 0), r2(reader, dataDay);
    w.join();
    r1.join();
    r2.join();
    auto stopWriterReader = std::chrono::high_resolution_clock::now();
    auto durationWriterReader = std::chrono::duration_cast<std::chrono::microseconds>(stopWriterReader - startWriterReader);
    std::cout << "Writer/Reader mode in time: " << durationWriterReader.count() << "\n";
    std::cout << "Done in Writer-Reader mode\n";

    return 0;
}
