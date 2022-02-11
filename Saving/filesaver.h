#ifndef FILESAVER_H
#define FILESAVER_H

#include <QDialog>



QT_BEGIN_NAMESPACE

namespace Ui {
class FileSaver;
}

QT_END_NAMESPACE


class FileSaver : public QDialog
{
    Q_OBJECT

public:
    explicit FileSaver(QWidget *parent = nullptr);
    ~FileSaver();

public:
    void saveFile(const QString &path, const QString &fileName, const std::vector<float> &data);
private:
    Ui::FileSaver *ui = nullptr;
};

#endif // FILESAVER_H
