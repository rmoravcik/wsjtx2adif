#include <QCoreApplication>
#include <QDebug>

#include "qsolog.h"
#include "adifenums.h"
#include "adiflogwriter.h"

QString wsjtxFileName = "ALL.TXT";
QString adifEnumsDbPath = "adif-enums.db";
QString qsoLogDbPath = "qso-log.db";
QString adifFileName = "wsjtx_log.adi";

int init(void)
{
    // file permissions for db files
    QFile::Permissions perm = QFile::WriteOwner | QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther;

    // remove any existing log db file
    // TODO eventually only do this if there were DB changes
    if(QFile::exists(qsoLogDbPath) && !QFile::remove(qsoLogDbPath)) {
        qWarning() << "Main: couldn't delete existing QSO log database";
    }

    // copy empty qso log if no log db file exists
    if(!QFile::exists(qsoLogDbPath)) {
        if(!QFile::copy(":/db/empty-qsolog", qsoLogDbPath)) {
            qCritical() << "Main: couldn't copy empty QSO log database to filesystem";
            return 1;
        } else {
            // set correct file permissions
            QFile logDb(qsoLogDbPath);

            if(!logDb.setPermissions(perm)) {
                qCritical() << "Main: couldn't set permissions on empty QSO log database";
                logDb.remove();
                return 1;
            }
        }
    }

    // remove any existing enum db file
    // TODO eventually only do this if there were DB changes
    if(QFile::exists(adifEnumsDbPath) && !QFile::remove(adifEnumsDbPath)) {
        qWarning() << "Main: couldn't delete existing ADIF enums database";
    }

    // copy enum db file out
    if(!QFile::copy(":/db/adifenums", adifEnumsDbPath)) {
        qCritical() << "Main: couldn't copy new ADIF enums database to filesystem";
        return 1;
    } else {
        // copy success; set correct permissions
        QFile adifDb(adifEnumsDbPath);

        if(!adifDb.setPermissions(perm)) {
            qCritical() << "Main couldn't set permissions on ADIF Enums database";
            adifDb.remove();
            return 1;
        }
    }

    // init QSO log
    if(!log::QsoLog::init(qsoLogDbPath.toStdString())) {
        qCritical() << "Main: QSO log init failed";
        return 1;
    }

    // init ADIF enums
    if(!adif::AdifEnums::init(adifEnumsDbPath.toStdString())) {
        qCritical() << "Main: ADIF enums init failed";
        return 1;
    }

    return 0;
}

int addQso(QString callsign, QDateTime time, QString band, QVariant freq, QString mode, QString message)
{
    log::Qso record;

    record.callsign = callsign;
    record.timeOnUtc = time;
    record.band = band;
    record.mode = mode;
    record.freqMhz = freq;
    record.qsoMsg = message;

    if (!log::QsoLog::addQso(record)) {
        qCritical() << "Main Log: Failed to log QSO record!";
        return 1;
    } else {
        qDebug() << "Main Log: Successfully logged QSO record!";
    }

    return 0;
}

int parseLog(void)
{
    QFile file(wsjtxFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Main Log: Failed to open" << wsjtxFileName;
        return 1;
    } else {
        qDebug() << "Main Log: Successfully opened log file";
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine();
        QDateTime date = QDateTime::fromString(line.mid(0, 13), "yyMMdd_hhmmss").addYears(100);
        QVariant freq = QVariant(line.mid(14, 9).simplified().toDouble());
        QString mode = line.mid(27, 6).simplified();
        QString message = line.mid(48).simplified();
        QString callsign = message.split(" ")[1];
        QString band = freq.toString();

        addQso(callsign, date, band, freq, mode, message);
    }

    file.close();
    return 0;
}

int exportToAdif(void)
{
    log::QsoLog::QsoList qsoList = log::QsoLog::getQsoList();

    adif::AdifLogWriter writer(adifFileName);
    if (!writer.write(qsoList)) {
        qCritical() << "Export: failure writing full log";
        return 1;
    } else {
        qDebug() << "Export: successfully wrote ADIF log export to " << adifFileName;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (init()) {
        return -1;
    }

    if (parseLog()) {
        return -1;
    }

    if (exportToAdif()) {
        return -1;
    }

    // cleanup
    log::QsoLog::destroy();
    adif::AdifEnums::destroy();

    return 0;
}
