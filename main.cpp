#include <QCoreApplication>
#include <QFile>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QDebug>

QString wsjtxFileName = "ALL.TXT";
QString adifFileName = "wsjtx_log.adi";

QStringList callsigns;

bool init(void)
{
    if (QFile::exists(adifFileName) && !QFile::remove(adifFileName)) {
        qWarning() << "Main: couldn't delete existing ADIF database";
        return false;
    }

    callsigns.clear();

    return true;
}

bool isGridValid(QString grid)
{
    if (grid.length() != 4) {
        return false;
    }
    return true;
}

bool exportToAdif(void)
{
    QFile inFile(wsjtxFileName);
    if (!inFile.open(QIODevice::ReadOnly)) {
        qCritical() << "wsjtx2adif: Failed to open" << wsjtxFileName;
        return false;
    }

    QFile outFile(adifFileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "wsjtx2adif: Failed to open" << adifFileName;
        return false;
    }

    QTextStream inStream(&inFile);
    QTextStream outStream(&outFile);

    outStream << "WSJT-X ADIF EXPORT<eoh>\n";

    while (!inStream.atEnd()) {
        QString line = inStream.readLine();
        QString freq = line.mid(14, 9).simplified();
        QString mode = line.mid(27, 6).simplified();
        QStringList message = line.mid(48).simplified().split(" ");
        QString callsign = message[1].simplified();
        if (message.count() > 2) {
            QString grid = message[2].simplified();

            if (isGridValid(grid)) {
                if (!callsigns.contains(callsign)) {
                    callsigns.append(callsign);
                    // <call:6>2E0FQT <gridsquare:4>IO93 <mode:3>FT8 <eor>
                    QString record = QString("<call:%1>%2 <gridsquare:4>%3 <eor>\n").arg(callsign.length()).arg(callsign).arg(grid);
                    outStream << record;
                }
            }
        }
    }

    inFile.close();

    outFile.flush();
    outFile.close();

    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (!init()) {
        return -1;
    }

    if (!exportToAdif()) {
        return -1;
    }

    return 0;
}
