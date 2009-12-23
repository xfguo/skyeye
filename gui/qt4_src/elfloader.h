#ifndef ELFLOADER_H
#define ELFLOADER_H

#include <QObject>

class ElfLoader : public QObject
{
public:
    ElfLoader();
    static bool checkFormat(QString fileName);
};

#endif // ELFLOADER_H
