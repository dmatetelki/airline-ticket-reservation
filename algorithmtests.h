#ifndef ALGORITHMTESTS_H
#define ALGORITHMTESTS_H

#include <QtTest/QtTest>

class AlgorithmTests : public QObject
{
    Q_OBJECT

public:

    explicit AlgorithmTests(QObject *parent = 0);

private slots:

    void testSeat();
    void testFileDataBase();

};

#endif // ALGORITHMTESTS_H
