#ifndef CSVFILEHANDLE_H
#define CSVFILEHANDLE_H

#include <QString>
#include <QLocale>
#include <fstream>

template<class T>
class CSVFileHandle
{
public:
    CSVFileHandle() {
        values = QVector<T>(0);
        values.clear();
    }
    CSVFileHandle(int estimatedEntryCount) {
        values = QVector<T>(0);
        values.reserve(estimatedEntryCount);
        values.clear();
        qDebug() << "csvfilehandle: vector with " << values.size() << " entries generated";
    }

    void addValue(T v) {
        values.push_back(v);
    }

    void flush(QString toFilePath) {
        std::ofstream myfile;
        myfile.open (toFilePath.toStdString());
        qDebug() << "csvfilehandle: vector with " << values.size() << " entries will get flushed";

        int i = 0;
        myfile << "entry count : " << values.size() << "\n";
        while( !values.isEmpty() ) {
            QLocale eng(QLocale::English);
            QString s1 = eng.toString(values.takeFirst());
            myfile << s1.toStdString() << "\n";
            //qDebug() << i ;
            i++;
        }
        myfile.close();
    }

private:
    QVector<T> values;
};



#endif // CSVFILEHANDLE_H
