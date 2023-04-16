#include <math.h>
#include <QCoreApplication>
#include <QVector>
#include <iostream>
#include <thread>
#include <QMutex>
#include <QWaitCondition>
#include <chrono>
#include <unistd.h>
#include <QWriteLocker>

QMutex mutex;
const unsigned totalBufferSize = 60 * 60 * 24 * 30;
const unsigned total = 60 * 60 * 24 * 365;  // sec * min * hour * days
const unsigned dataDay = 60 * 60 * 24;
QWaitCondition buffertNotEmpty;
QWaitCondition bufferNotFull;
QVector<float> Buffer;
unsigned comparator = 0;

class Statistics {
  // https://stackoverflow.com/questions/7988486/how-do-you-calculate-the-variance-median-and-standard-deviation-in-c-or-java/7988556#7988556
  QVector<float> data;
  int size;

 public:
  Statistics(QVector<float> data) {
    this->data = data;
    size = data.length();
  }

  double getMean() {
    double sum = 0.0;
    for (double a : data) sum += a;
    return sum / size;
  }

  double median() {
    // Arrays.sort(data);
    std::sort(data.begin(), data.end());
    if (data.length() % 2 == 0)
      return (data[(data.length() / 2) - 1] + data[data.length() / 2]) / 2.0;
    return data[data.length() / 2];
  }
};

void serialMode(QVector<float> d, unsigned int total)
{
    for (unsigned int i = 0; i < total; i++) {
      d.push_back(random() % 50 + 50);  // generate a number between 50 and 99
      if (i % (24 * 3600) == 0) {
        Statistics s(d);
        std::cout << i << " of " << total << " ";
        std::cout << "average temperature " << s.getMean() << " ";
        std::cout << " with median " << s.median() << "\n";
        d.erase(d.begin(), d.end());
      }
    }
}

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

void producer()
{
    for (unsigned int i = 0; i < total; i++) {
        mutex.lock();
        if (comparator == totalBufferSize) {
            bufferNotFull.wait(&mutex);
        }
        mutex.unlock();

        Buffer[i % totalBufferSize] = random() % 50 + 50;
        comparator++;

        mutex.lock();
        if (comparator >= dataDay * 2) {
            buffertNotEmpty.wakeAll();
        }
        mutex.unlock();
    }
}

void consumer(unsigned int startPos)
{
    unsigned int count = startPos;
    while (count < total) {
        mutex.lock();
        if (comparator < dataDay) {
            buffertNotEmpty.wait(&mutex);
        }
        mutex.unlock();

        std::cout << count << " of " << total << " ";
        std::cout << "average temperature " << getMeanConsumer(&Buffer, count % totalBufferSize) << " ";
        std::cout << " with median " << getMedianConsumer(&Buffer, count % totalBufferSize) << "\n";

        mutex.lock();
        comparator = comparator - dataDay;
        bufferNotFull.wakeAll();
        mutex.unlock();

        count += dataDay * 2;
    }
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
    QVector<float> d;
    Buffer.resize(total);

    auto startSerial = std::chrono::high_resolution_clock::now();
    serialMode(d, total);
    auto stopSerial = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> durationSerial = stopSerial - startSerial;
    std::cout << "Done in serial mode\n\n\n";
    float durationSerialSeconds = durationSerial.count();

    auto startProducerConsumer = std::chrono::high_resolution_clock::now();
    std::thread p(producer), c1(consumer, 0), c2(consumer, dataDay);
    p.join();
    c1.join();
    c2.join();
    auto stopProducerConsumer = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> durationProducerConsumer = stopProducerConsumer - startProducerConsumer;
    std::cout << "Done in producer-consumer mode\n\n\n";
    float durationProducerConsumerSeconds = durationProducerConsumer.count();

    auto startWriterReader = std::chrono::high_resolution_clock::now();
    std::thread w(writer), r1(reader, 0), r2(reader, dataDay);
    w.join();
    r1.join();
    r2.join();
    auto stopWriterReader = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> durationWriterReader = stopWriterReader- startWriterReader;
    std::cout << "Done in Writer-Reader mode\n\n\n";
    float durationReaderWriterSeconds = durationWriterReader.count();

    std::cout << "\nSerial mode in time: " << durationSerialSeconds << "\n";
    std::cout << "Producer/Consumer mode in time: " << durationProducerConsumerSeconds << "\n";
    std::cout << "Writer/Reader mode in time: " << durationReaderWriterSeconds << "\n";
}
