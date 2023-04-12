#include <math.h>
#include <QCoreApplication>
#include <QVector>
#include <iostream>
#include <thread>
#include <QMutex>
#include <QWaitCondition>

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

  double getMeanConsumer(QVector<float>* data, unsigned startPos) {
    double sum = 0.0;
    for (unsigned i = startPos; i < startPos + dataDay; i++) {
        sum += (*data)[i % totalBufferSize];
    }
    return sum / dataDay;
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
        std::cout << " with median " << "\n";
        d.erase(d.begin(), d.end());
      }
    }
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

        Statistics s(Buffer);
        std::cout << count << " of " << total << " ";
        std::cout << "average temperature " << s.getMeanConsumer(&Buffer, count % totalBufferSize) << " ";
        std::cout << " with median " << "\n";
        Buffer.erase(Buffer.begin(), Buffer.end());

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
    std::thread p(producer);
    std::thread c1(consumer, 0), c2(consumer, dataDay);

    p.join();
    c1.join();
    c2.join();
    std::cout << "Done in producer-consumer mode\n";

    return 0;
}
