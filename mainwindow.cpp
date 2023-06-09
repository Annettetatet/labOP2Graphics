#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>

#define AXIS_MARGIN_X       50
#define AXIS_MARGIN_Y       50
#define AXIS_ARROW_LENGTH   15
#define GRAPH_BORDER        5
#define TEXT_MARGIN_LEFT    5
#define TEXT_MARGIN_TOP     5


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

char* QstringToCharArray(QString qstr)
{
    char *str = (char*)malloc(sizeof(char)*(qstr.size() + 1));
    size_t i;
    for (i = 0; i < qstr.size(); i++)
    {
        str[i] = qstr.at(i).unicode();
    }
    str[i] = 0;
    return str;
}

QStringList ConvertRowToQTFormat(char **row, size_t size)
{
    QStringList qsl = {};

    for(size_t i = 0; i < size; i++)
    {
        qsl.append(QString::fromUtf8(row[i]));
    }

    return qsl;
}

void MainWindow::showData(FuncReturningValue* frv)
{
    ui->tableWidget->setColumnCount(frv->fields_num);
    QStringList QColumns = ConvertRowToQTFormat(frv->headers, frv->fields_num);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QStringList QNumberedColumns = QStringList();
    for(int i=0; i< QColumns.size(); i++) {
        QNumberedColumns.append(QString::number(i + 1).append(" ").append(QColumns.at(i)));
    }

    ui->tableWidget->setHorizontalHeaderLabels(QNumberedColumns);
    if (frv->data != NULL)
    {
        ui->tableWidget->setRowCount(0);
        for (size_t i = 0; i < frv->len; i++)
        {
            QStringList currentRow = ConvertRowToQTFormat(frv->data[i], frv->fields_num);
            ui->tableWidget->setRowCount(i + 1);
            for (int j = 0; j < currentRow.count(); j++)
            {
                QTableWidgetItem *item = new QTableWidgetItem();
                item->setData(Qt::EditRole, currentRow.at(j).toDouble());
                item->setText(currentRow.at(j));
                ui->tableWidget->setItem(i, j, item);
            }
        }
    }
}


void MainWindow::on_selectFileButton_clicked()
{
    this->filename = QFileDialog::getOpenFileName(this, tr("Open File"), NULL, "CSV File (*.csv)");

    bool isFileSelected = QFile::exists(this->filename);

    if (isFileSelected) {
        ui->filenameLabel->setText(this->filename);
        ui->selectFileButton->setEnabled(false);
        ui->loadDataButton->setEnabled(true);
        ui->clearBtn->setEnabled(true);

        ui->regionEdit->setEnabled(true);
        ui->calcColumnEdit->setEnabled(true);
    } else {
        ui->loadDataButton->setEnabled(false);
        ui->calculateMetricsButton->setEnabled(false);

        ui->filenameLabel->setText("Select file...");
        ui->tableWidget->clear();
        ui->resultPlainTextEdit->clear();
    }
}

void MainWindow::returnWithWarning(QString warning)
{
    QMessageBox::warning(this, tr("Warning"),
                                   tr("Region name is empty"),
                                   QMessageBox::Ok);
}

FuncReturningValue* MainWindow::loadData(QString filename, QString regionName, int columnRegion, int calcColumnNumber)
{
    FuncArgument fa = {
        .filename = QstringToCharArray(filename),
        .region_name = QstringToCharArray(regionName),
        .column_for_region = columnRegion,
        .column_to_calc = calcColumnNumber
    };
    return entryPoint(getData, &fa);
}

void MainWindow::showDataAndClean(FuncReturningValue* frv)
{
    int columnRegion = frv->column_for_region;
    ui->regionColumnNumberLbl->setText(QString::number(columnRegion));

    if (frv->len > 0)
        showData(frv);

    FuncArgument fa2 = {
        .data = frv->data,
        .headers = frv->headers,
        .data_len = frv->data_len,
        .len = frv->len,
        .fields_num = frv->fields_num
    };
    entryPoint(cleanData, &fa2);
    free(frv);
}

void MainWindow::on_loadDataButton_clicked()
{
    QString regionName = ui->regionEdit->text();
    if (regionName.isEmpty())
        return returnWithWarning(tr("Region name is empty"));

    QString calcColumnText = ui->calcColumnEdit->text();
    if (calcColumnText.isEmpty())
        return returnWithWarning(tr("Calculate column number is empty"));

    bool calcColumnNumberOk = false;
    int calcColumnNumber = calcColumnText.toInt(&calcColumnNumberOk);
    if (!calcColumnNumber || calcColumnNumber < 1)
        return returnWithWarning(tr("Calculate column number is invalid"));

    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);
    ui->resultPlainTextEdit->clear();
    ui->graphLabel->clear();

    FuncReturningValue* frv = loadData(this->filename, regionName, columnRegion, calcColumnNumber);

    if (frv->isRegionColumnNotFound)
        return returnWithWarning(tr("Region column not found"));
    if (frv->isNonNumberInCalcField)
        return returnWithWarning(tr("Non-numeric values in calculation field"));

    showDataAndClean(frv);

    ui->calculateMetricsButton->setEnabled(true);
}

void MainWindow::on_clearBtn_clicked()
{
    ui->filenameLabel->setText("Select file...");
    ui->selectFileButton->setEnabled(true);
    ui->loadDataButton->setEnabled(false);
    ui->calculateMetricsButton->setEnabled(false);
    ui->clearBtn->setEnabled(false);

    ui->regionColumnNumberLbl->setText("-");
    ui->regionEdit->setEnabled(false);
    ui->calcColumnEdit->setEnabled(false);

    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);
    ui->resultPlainTextEdit->clear();
    ui->graphLabel->clear();
}


char*** MainWindow::getDataFromTable()
{
    char ***data = (char ***)malloc(sizeof(char**) * ui->tableWidget->rowCount());
    for (size_t i = 0; i < (size_t)ui->tableWidget->rowCount(); i++)
    {
        data[i] = (char **)malloc(sizeof(char*) * ui->tableWidget->columnCount());
        for (size_t j = 0; j < (size_t)ui->tableWidget->columnCount(); j++)
        {
            //Получаем значение в i-ой строке и j-ом столбце
            QTableWidgetItem *item = ui->tableWidget->item(i,j);\
            //Приводим значение ячейки к стандартному типу строки
            char* str = QstringToCharArray(item->text());
            data[i][j] = str;
        }
    }
    return data;
}

void renderGraph(QPainter *paint, int SIZE_X, int SIZE_Y,
                 FuncReturningValue* frv, int stepX, int stepY)
{
    paint->setPen(Qt::GlobalColor::blue);

    for(int i=1; i < frv->result_rows; i++) {
        double prevValue = frv->result_array[i-1]->value;
        double curValue = frv->result_array[i]->value;

        int prevValueScalled = (prevValue - frv->result_min) * stepY;
        int curValueScalled = (curValue - frv->result_min) * stepY;

        paint->drawLine(AXIS_MARGIN_X + GRAPH_BORDER + (i-1) * stepX,
                       SIZE_Y - AXIS_MARGIN_Y - GRAPH_BORDER - prevValueScalled,
                       AXIS_MARGIN_X + GRAPH_BORDER + i * stepX,
                       SIZE_Y - AXIS_MARGIN_Y - GRAPH_BORDER - curValueScalled);

        paint->rotate(90);
        paint->drawText(SIZE_Y - AXIS_MARGIN_Y + TEXT_MARGIN_TOP * 4,
                       -AXIS_MARGIN_X - GRAPH_BORDER - (i - 1) * stepX,
                       QString(frv->result_array[i-1]->label));
        paint->rotate(-90);
    }
}

void renderCalcValues(QPainter *paint, int SIZE_X, int SIZE_Y,
                      FuncReturningValue* frv, int stepX, int stepY,
                      int minValueScalled, int maxValueScalled, int medianValueScalled)
{
    paint->setPen(Qt::GlobalColor::red);

    paint->drawLine(AXIS_MARGIN_X, minValueScalled, SIZE_X, minValueScalled);
    paint->drawLine(AXIS_MARGIN_X, maxValueScalled, SIZE_X, maxValueScalled);
    paint->drawLine(AXIS_MARGIN_X, medianValueScalled, SIZE_X, medianValueScalled);
}

void renderAxis(QPainter *paint, int SIZE_X, int SIZE_Y, int minValueScalled,
                int maxValueScalled, int medianValueScalled)
{
    // отрисовка осей
    paint->setPen(Qt::GlobalColor::black);

    // ось y
    paint->drawLine(AXIS_MARGIN_X, 0, AXIS_MARGIN_X, SIZE_Y - AXIS_MARGIN_Y);
    paint->drawLine(AXIS_MARGIN_X, 0, AXIS_MARGIN_X - AXIS_ARROW_LENGTH, AXIS_ARROW_LENGTH);
    paint->drawLine(AXIS_MARGIN_X, 0, AXIS_MARGIN_X + AXIS_ARROW_LENGTH, AXIS_ARROW_LENGTH);

    // подписи y
    paint->rotate(90);
    paint->drawText(AXIS_ARROW_LENGTH, -AXIS_MARGIN_X + TEXT_MARGIN_LEFT * 2, "values");
    paint->rotate(-90);

    paint->drawText(TEXT_MARGIN_LEFT, minValueScalled + TEXT_MARGIN_TOP, "min");
    paint->drawText(TEXT_MARGIN_LEFT, maxValueScalled + TEXT_MARGIN_TOP * 2, "max");
    paint->drawText(TEXT_MARGIN_LEFT, medianValueScalled + TEXT_MARGIN_TOP, "median");

    // ось x
    paint->drawLine(AXIS_MARGIN_X, SIZE_Y - AXIS_MARGIN_Y,
                   SIZE_X, SIZE_Y - AXIS_MARGIN_Y);
    paint->drawLine(SIZE_X - AXIS_ARROW_LENGTH, SIZE_Y - AXIS_MARGIN_Y - AXIS_ARROW_LENGTH,
                   SIZE_X, SIZE_Y - AXIS_MARGIN_Y);
    paint->drawLine(SIZE_X - AXIS_ARROW_LENGTH, SIZE_Y - AXIS_MARGIN_Y + AXIS_ARROW_LENGTH,
                   SIZE_X, SIZE_Y - AXIS_MARGIN_Y);

    // подписи y
    paint->drawText(SIZE_X - 50, SIZE_Y - AXIS_MARGIN_Y + TEXT_MARGIN_TOP * 3, "dates");
}

void MainWindow::drawGraph(FuncReturningValue* frv)
{
    const int SIZE_X = ui->graphLabel->minimumSizeHint().width(), SIZE_Y = ui->graphLabel->minimumSizeHint().height();
    int stepX = (SIZE_X - AXIS_MARGIN_X - GRAPH_BORDER) / frv->result_rows;
    int stepY = (SIZE_Y - AXIS_MARGIN_Y - GRAPH_BORDER) / (frv->result_max - frv->result_min);

    QPixmap *pix = new QPixmap(SIZE_X, SIZE_Y);
    QPainter paint(pix);
    paint.fillRect(0, 0, SIZE_X, SIZE_Y, QBrush(QColor(Qt::GlobalColor::white)));

    renderGraph(&paint, SIZE_X, SIZE_Y, frv, stepX, stepY);

    int minValueScalled = SIZE_Y - AXIS_MARGIN_Y - GRAPH_BORDER - (frv->result_min - frv->result_min) * stepY;
    int maxValueScalled = SIZE_Y - AXIS_MARGIN_Y - GRAPH_BORDER - (frv->result_max - frv->result_min) * stepY;
    int medianValueScalled = SIZE_Y - AXIS_MARGIN_Y - GRAPH_BORDER - (frv->result_median - frv->result_min) * stepY;

    renderCalcValues(&paint, SIZE_X, SIZE_Y, frv, stepX, stepY,
                     minValueScalled, maxValueScalled, medianValueScalled);

    renderAxis(&paint, SIZE_X, SIZE_Y, minValueScalled,
               maxValueScalled, medianValueScalled);

    ui->graphLabel->setPixmap(*pix);
}

void MainWindow::on_calculateMetricsButton_clicked()
{
    bool columnNumberOk = false;
    QString columnNumberText = ui->calcColumnEdit->text();
    int columnNumber = columnNumberText.toInt(&columnNumberOk);
    FuncArgument fa = {
        .data = getDataFromTable(),
        .len = (size_t)ui->tableWidget->rowCount(),
        .fields_num = (size_t)ui->tableWidget->columnCount(),
        .column_for_region = columnRegion,
        .column_to_calc = columnNumber
    };
    FuncReturningValue* frv = entryPoint(calculateData, &fa);
    ui->resultPlainTextEdit->clear();
    if (frv->result_rows > 0) {
        ui->resultPlainTextEdit->appendPlainText(
                    QString("Minimum: %1\nMaximum: %2\nMedian: %3\nRows: %4")
                    .arg(frv->result_min)
                    .arg(frv->result_max)
                    .arg(frv->result_median)
                    .arg(frv->result_rows)
                    );

        drawGraph(frv);
    } else {
        ui->resultPlainTextEdit->appendPlainText(QString("Rows not found by region name"));
    }
    entryPoint(cleanData, &fa);
    free(frv);
}
