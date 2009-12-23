#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>

#define MAX_STR_NAME 255


class Console : public QPlainTextEdit
{
    Q_OBJECT
public:
    Console();
    /*
    int consoleOpen(struct uart_device *uart_dev);
    int consoleClose(struct uart_device *uart_dev);
    int consoleRead(struct uart_device *uart_dev, void *buf, size_t count,struct timeval *timeout);
    int consoleWite(struct uart_device *uart_dev, void *buf, size_t count);
    */

};

#endif // CONSOLE_H
