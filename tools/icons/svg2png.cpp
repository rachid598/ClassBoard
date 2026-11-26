// Petit utilitaire : rend une icône SVG ClassBoard (currentColor) en PNG,
// sur une pastille arrondie colorée. Sert aux icônes des widgets (.wgt).
//   svg2png <entrée.svg> <sortie.png> <taille> <couleur fond> <couleur trait>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);
    if (argc < 6)
        return 1;

    QFile file(argv[1]);
    if (!file.open(QIODevice::ReadOnly))
        return 2;

    QByteArray svg = file.readAll();
    svg.replace("currentColor", argv[5]);

    const int size = QByteArray(argv[3]).toInt();
    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(argv[4]));
    painter.drawRoundedRect(QRectF(0, 0, size, size), size * 0.22, size * 0.22);

    QSvgRenderer renderer(svg);
    const qreal glyph = size * 0.6;
    renderer.render(&painter, QRectF((size - glyph) / 2, (size - glyph) / 2, glyph, glyph));
    painter.end();

    return image.save(argv[2]) ? 0 : 3;
}
