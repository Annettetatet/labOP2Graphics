#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "applogic.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_selectFileButton_clicked();

    void on_loadDataButton_clicked();

    void on_calculateMetricsButton_clicked();

    void on_clearBtn_clicked();

private:
    Ui::MainWindow *ui;

    QString filename;
    int columnRegion;
    void showData(FuncReturningValue*);
    char*** getDataFromTable();
    void drawGraph(FuncReturningValue* frv);

    void returnWithWarning(QString warning);
    FuncReturningValue* loadData(QString filename, QString regionName, int columnRegion, int calcColumnNumber);
    void showDataAndClean(FuncReturningValue* frv);
};
#endif // MAINWINDOW_H
