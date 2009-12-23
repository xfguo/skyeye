#ifndef MACHCONF_H
#define MACHCONF_H

#include <QObject>
#include <QIODevice>
#include <QDomDocument>

class MachConf : public QObject
{
    Q_OBJECT
public:
    MachConf();
    MachConf(QString fileName);
    static bool parseConf(QString fileName);
    bool read();

private:
    QDomDocument domDocument;
    QString _fileName;
};

#endif // MACHCONF_H
