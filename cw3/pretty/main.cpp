#include <QApplication>
#include <QImage>
#include <QFile>
#include <QtDebug>
#include <QLabel>

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

  QFile file("foo");
  file.open(QIODevice::ReadOnly);
  QDataStream s(&file);

  QImage image(1000, 1000, QImage::Format_ARGB32);

  QRgb* p = reinterpret_cast<QRgb*>(image.bits());
  quint8 c;
  while (!s.atEnd()) {
    s >> c;
    *p = colors[c];
    p++;
  }

  QLabel label;
  label.setPixmap(QPixmap::fromImage(image));
  label.setWindowTitle("File");
  label.show();

  QImage image2(1000, 1000, QImage::Format_ARGB32);
  p = reinterpret_cast<QRgb*>(image2.bits());
  for (int i=0 ; i<1000000 ; ++i) {
    *p = colors[qrand() % 7];
    p++;
  }

  QLabel label2;
  label2.setPixmap(QPixmap::fromImage(image2));
  label2.setWindowTitle("Random");
  label2.show();

  return a.exec();
}
