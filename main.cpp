#include <math.h>
#include <QCoreApplication>
#include <QVector>
#include <iostream>
#include <thread>
#include <QMutex>
#include <QWaitCondition>
#include <chrono>
#include <unistd.h>

QMutex mutex;
const unsigned totalBufferSize = 60 * 60 * 24 * 30;
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

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QVector<float> d;
    unsigned int total = 60 * 60 * 24 * 365;  // sec * min * hour * days
    unsigned int dataDay = 60 * 60 * 24;
    int option;
    Buffer.resize(total);

    auto startSerial = std::chrono::high_resolution_clock::now();
    serialMode(d, total);
    auto stopSerial = std::chrono::high_resolution_clock::now();
    auto durationSerial = std::chrono::duration_cast<std::chrono::microseconds>(stopSerial - startSerial);
    std::cout << "Serial mode in time: " << durationSerial.count() << "\n";
    std::cout << "Done in serial mode\n";
}
