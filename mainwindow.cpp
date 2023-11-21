#include "mainwindow.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QStatusBar>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPushButton>
#include <QTimer>
#include <QSettings>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    createActions();

    choice1 = new QCheckBox("Choice 1", this);
    statusBar()->addPermanentWidget(choice1);

    comboBoxPort = new QComboBox(this);
    comboBoxBaudRate = new QComboBox(this);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        comboBoxPort->addItem(info.portName());
    }

    comboBoxBaudRate->addItem("9600");
    comboBoxBaudRate->addItem("19200");
    comboBoxBaudRate->addItem("115200");

    comboBoxBaudRate->setCurrentIndex(3);

    comboBoxInterval = new QComboBox(this);
    comboBoxInterval->addItem("10 ms");
    comboBoxInterval->addItem("50 ms");
    comboBoxInterval->addItem("100 ms");


    statusBar()->addPermanentWidget(comboBoxPort);
    statusBar()->addPermanentWidget(comboBoxBaudRate);
    statusBar()->addPermanentWidget(comboBoxInterval);

    connect(choice1, &QCheckBox::clicked, this, &MainWindow::onChoice1Clicked);
    connect(comboBoxInterval, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onIntervalChanged);

    dataTimer = new QTimer(this); //???
    connect(dataTimer, &QTimer::timeout, this, &MainWindow::sendData); //???

    saveButton = new QPushButton("Save setting txt", this);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveSettings);
    statusBar()->addPermanentWidget(saveButton);

    sendBuffHistoryButton = new QPushButton("Send Buff History", this);
    connect(sendBuffHistoryButton, &QPushButton::clicked, this, &MainWindow::sendBuffHistory);
    statusBar()->addPermanentWidget(sendBuffHistoryButton);

}



void MainWindow::onChoice1Clicked() {
    if (choice1->isChecked()) {
        QString selectedPort = comboBoxPort->currentText();
        int baudRate = comboBoxBaudRate->currentText().toInt();

        QSerialPort serialPort;
        serialPort.setPortName(selectedPort);
        serialPort.setBaudRate(baudRate);

        if (serialPort.open(QIODevice::WriteOnly)) {
            QString dataToSend = "Ваші дані для відправки";
            serialPort.write(dataToSend.toUtf8());
            serialPort.close();

            int interval = comboBoxInterval->currentText().split(" ")[0].toInt();
            dataTimer->setInterval(interval);

            dataTimer->start();
        } else {
        }
    }
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout;

    tableWidget = new QTableWidget(this);
    mainLayout->addWidget(tableWidget);

    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *openAction = fileMenu->addAction("Open CSV File");
    connect(openAction, &QAction::triggered, this, &MainWindow::openCSVFile);

   sendButton = new QPushButton("Send Data", this);
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    statusBar()->addPermanentWidget(sendButton);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void MainWindow::onSendButtonClicked() {
    QString selectedPort = comboBoxPort->currentText();
    int baudRate = comboBoxBaudRate->currentText().toInt();

    QSerialPort serialPort;
    serialPort.setPortName(selectedPort);
    serialPort.setBaudRate(baudRate);

    if (serialPort.open(QIODevice::WriteOnly)) {
        QList<QTableWidgetItem *> selectedItems = tableWidget->selectedItems();
        for (QTableWidgetItem *item : selectedItems) {
            QString dataToSend = item->text();
            serialPort.write(dataToSend.toUtf8());
        }

        serialPort.close();
    } else {
    }
}


void MainWindow::onIntervalChanged() {
    if (dataTimer->isActive()) {
        int interval = comboBoxInterval->currentText().split(" ")[0].toInt();
        dataTimer->setInterval(interval);
    }
}

void MainWindow::createActions() {
    setupUI();

    connect(this, &MainWindow::destroyed, qApp, &QApplication::quit);
}

void MainWindow::saveSettings() { //txt
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Settings"), "", tr("Text Files (*.txt);;All Files (*)"));

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out << "Port Name: " << comboBoxPort->currentText() << "\n";
    out << "Baud Rate: " << comboBoxBaudRate->currentText() << "\n";
    out << "Interval: " << comboBoxInterval->currentText() << "\n";

    file.close();
}

void MainWindow::openCSVFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open CSV File", QDir::homePath(), "svc_file (*.csv)");

    if (!filePath.isEmpty()) {
        statusBar()->showMessage("Opening CSV file: " + filePath);

        loadCSVData(filePath);

        if (choice1->isChecked()) {
            QList<QTableWidgetItem *> selectedItems = tableWidget->selectedItems();
            for (QTableWidgetItem *item : selectedItems) {
                QString dataToSend = item->text();
                sendDataViaSerial(dataToSend.toFloat()); // передача даних у вигляді float
            }
        } else {
        }
    }
}


void MainWindow::loadCSVData(const QString &filePath) {
    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        tableWidget->clear();
        tableWidget->setRowCount(0);
        tableWidget->setColumnCount(0);

        QString headerLine = in.readLine();
        QStringList headerFields = headerLine.split(';');

        tableWidget->setColumnCount(headerFields.size());
        tableWidget->setHorizontalHeaderLabels(headerFields);

        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(';');

            int row = tableWidget->rowCount();
            tableWidget->insertRow(row);

            if (tableWidget->columnCount() < fields.size()) {
                tableWidget->setColumnCount(fields.size());
            }

            for (int i = 0; i < fields.size(); ++i) {
                QTableWidgetItem *newItem = new QTableWidgetItem(fields[i]);
                // Заборона редагування комірок
                newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
                tableWidget->setItem(row, i, newItem);
            }
        }
        file.close();
    }
}

void MainWindow::sendData() {
    QList<QTableWidgetItem *> selectedItems = tableWidget->selectedItems();
    for (QTableWidgetItem *item : selectedItems) {
        QString dataToSend = item->text();
        QStringList floatValues = dataToSend.split(",");

        // Перетворення рядків на числа типу float та відправлення через послідовний порт
        foreach (const QString &value, floatValues) {
            bool ok;
            float floatValue = value.toFloat(&ok);
            if (ok) {
                // Відправка числа типу float через послідовний порт
                sendDataViaSerial(floatValue);
            }
        }
    }
}
void MainWindow::sendDataViaSerial(float value) {
    QString selectedPort = comboBoxPort->currentText();
    int baudRate = comboBoxBaudRate->currentText().toInt();

    QSerialPort serialPort;
    serialPort.setPortName(selectedPort);
    serialPort.setBaudRate(baudRate);

    if (serialPort.open(QIODevice::WriteOnly)) {
        QByteArray dataToSend(reinterpret_cast<const char*>(&value), sizeof(value));
        serialPort.write(dataToSend);
        serialPort.close();
    } else {
    }
}

void MainWindow::sendBuffHistory() {
    QString selectedPort = comboBoxPort->currentText();
    int baudRate = comboBoxBaudRate->currentText().toInt();

    QSerialPort serialPort;
    serialPort.setPortName(selectedPort);
    serialPort.setBaudRate(baudRate);

    if (serialPort.open(QIODevice::WriteOnly)) {
        int selectedRow = tableWidget->currentRow();
        int columnIndex = 2;
        if (selectedRow >= 0) {
            QTableWidgetItem *buffHistoryItem = tableWidget->item(selectedRow, columnIndex);
            if (buffHistoryItem) {
                QString dataToSend = buffHistoryItem->text();
                serialPort.write(dataToSend.toUtf8());
            }
        }

        serialPort.close();
    } else {
    }
}

