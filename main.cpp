#include <math.h>

#include <QCoreApplication>
#include <QVector>
#include <iostream>

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
        s.getMean();
        s.median();
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
  int option;

  do {
    std::cout << " ==== Synchronization problems ==== " << std::endl;
    std::cout << "  1. Serial mode." << std::endl;
    std::cout << "  2. Producer-consumer mode." << std::endl;
    std::cout << "  3. Readers-writer mode." << std::endl;
    std::cout << "Choose a option: ";
    std::cin >> option;

    if (option < 1 || option > 3) {
      std::cout << "Invalid option. Try again." << std::endl << std::endl;
    }
  } while (option < 1 || option > 3);

  switch (option) {
    case 1:
        serialMode(d, total);
        std::cout << "Done in serial mode\n";
        break;
    case 2:
        std::cout << "Done in producer-consumer mode\n";
        break;
    case 3:
        std::cout << "Done in readers-writer mode\n";
        break;
    default:
        std::cout << "Invalid option\n";
        exit(EXIT_FAILURE);
        break;
  }
  return 0;
}
