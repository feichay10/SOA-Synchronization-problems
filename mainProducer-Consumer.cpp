#include <math.h>
#include <QCoreApplication>
#include <QVector>
#include <iostream>
#include <thread>
#include <QMutex>
#include <QWaitCondition>
#include <unistd.h>

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

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    Buffer.resize(total);
    std::thread p(producer), c1(consumer, 0), c2(consumer, dataDay);

    p.join();
    c1.join();
    c2.join();
    std::cout << "Done in producer-consumer mode\n";

    return 0;
}
