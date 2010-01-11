#include <QApplication>
#include <QImage>
#include <QFile>
#include <QtDebug>
#include <QLabel>

#include <cmath>

static const QRgb colors[] = {
  qRgb(255, 0, 0),
  qRgb(0, 255, 0),
  qRgb(0, 0, 255),
  qRgb(255, 255, 0),
  qRgb(255, 0, 255),
  qRgb(0, 255, 255),
  qRgb(255, 255, 255),
};

int main(int argc, char** argv) {
  QApplication a(argc, argv);

  QFile file(argv[1]);
  file.open(QIODevice::ReadOnly);
  QDataStream s(&file);

  const int size = sqrt(file.size()) + 1;

  QImage image(size, size, QImage::Format_ARGB32);
  image.fill(Qt::white);

  QRgb* p = reinterpret_cast<QRgb*>(image.bits());
  quint8 c;
  while (!s.atEnd()) {
    s >> c;
    if (c == '\n' || c == '\r')
      break;

    c -= '0';
    Q_ASSERT(c < 7);
    *p = colors[c];
    p++;
  }

  image.save("image.png");

  QLabel label;
  label.setPixmap(QPixmap::fromImage(image));
  label.setWindowTitle("File");
  label.show();

  return a.exec();
}
