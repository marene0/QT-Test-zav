#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QCheckBox>
#include <QSerialPort>
#include <QComboBox>
#include <QLayout>
#include <QPushButton>
#include <QSettings>
#include <QCloseEvent>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

//protected:
 //   void closeEvent(QCloseEvent *event) override;

private slots:
    void openCSVFile();
    void onChoice1Clicked();
    void onSendButtonClicked();
    void onIntervalChanged();
    void sendData();
    void sendBuffHistory();
   // void onSaveButtonClicked();

private:
    void setupUI();
    void createActions();
    void loadCSVData(const QString &filePath);
    void saveSettingstxt();
    void loadSettings();
    void sendDataViaSerial(float value);
    void saveSettings();

    QTableWidget *tableWidget;
    QCheckBox *choice1;
    QComboBox *comboBoxPort;
    QComboBox *comboBoxBaudRate;
    QComboBox *comboBoxInterval;
    QPushButton *sendButton;
    QTimer *dataTimer;
    QSerialPort serialPort;
    QPushButton *saveButton;
    QPushButton *sendBuffHistoryButton;

    QSettings settings;
};

#endif // MAINWINDOW_H

