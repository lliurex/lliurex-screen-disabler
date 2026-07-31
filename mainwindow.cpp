// SPDX-FileCopyrightText: 2026 Enrique M.G. <quique@necos.es>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.hpp"

extern "C" {
    #include <libdisplay-info/info.h>
    #include <libdisplay-info/edid.h>
}

#include <libintl.h>

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QIcon>
#include <QJsonDocument>
#include <QFile>
#include <QVariant>
#include <QDir>
#include <QRegularExpression>
#include <QStringList>
#include <QMessageBox>
#include <QApplication>
#include <QProcess>

#include <fstream>

using namespace lliurex;
using namespace std;

#define T(msg) dgettext ("lliurex-screen-disabler",msg)

const QStringList screenDB = {
    ".*L01N8A.*" /* small integrated on tower screen */
};

QString getVendorName(QString code)
{
    QString value = "Unknown";

    fstream f;
    f.open("/usr/share/hwdata/pnp.ids", std::ios::in);

    string line;

    while (f.good()) {
        std::getline(f,line);

        std::size_t tab = line.find('\t');
        if (tab == std::string::npos) {
            continue;
        }

        string c1 = line.substr(0,tab);
        string c2 = line.substr(tab);

        if (code.toStdString() == c1) {
            value = QString::fromStdString(c2);
        }
    }

    f.close();

    return value;
}

QString readEdid(QString path)
{
    QString value;

    vector<uint8_t> buffer;

    fstream f;
    std::string spath = path.toStdString();

    f.open(spath.c_str(),std::ios::binary | std::ios::in);

    while (f.good()) {
        char data;
        f.read(&data,1);
        buffer.push_back(data);
    }

    f.close();

    struct di_info* info;

    info = di_info_parse_edid((void *)buffer.data(),buffer.size());
    value = di_info_get_model(info);
    qDebug()<<"model:"<<value;

    const struct di_edid* edid = di_info_get_edid(info);

    const struct di_edid_vendor_product* product = di_edid_get_vendor_product(edid);

    QString manufacturer = getVendorName(product->manufacturer);

    qDebug()<<Qt::hex<<"vendor:"<<product->product;
    qDebug()<<"["<<product->manufacturer<<"]";
    qDebug()<<"["<<manufacturer<<"]";
    value = manufacturer + " " + value;
    di_info_destroy(info);

    return value;
}

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent)
{
    QWidget* centralWidget = new QWidget(this);
    m_layout = new QVBoxLayout(centralWidget);
    m_screenList = new QVBoxLayout();


    QHBoxLayout* toolbar = new QHBoxLayout();
    QPushButton* btn;

    toolbar->addStretch();
    btn = new QPushButton(T("Apply"));
    connect(btn, &QPushButton::clicked, [this]()
    {
        exportConfig();

        QMessageBox msgBox(this);
        msgBox.setText(T("Screen settings has been set successfully"));
        msgBox.exec();

        QApplication::quit();
    });
    toolbar->addWidget(btn);

    btn = new QPushButton(T("Cancel"));
    connect(btn, &QPushButton::clicked, [this]()
    {
        close();
    });
    toolbar->addWidget(btn);

    setWindowTitle("Lliurex Screen Disabler");
    resize(400, 300);

    m_layout->addWidget(new QLabel(T("Available monitors")));
    m_layout->addLayout(m_screenList);
    m_layout->addStretch();
    m_layout->addLayout(toolbar);
    setCentralWidget(centralWidget);

    loadConfig();
}

MainWindow::~MainWindow()
{

}

void MainWindow::loadConfig()
{
    QFile configFile;

    configFile.setFileName(QDir::home().path()+"/.config/kwinoutputconfig.json");

    if (!configFile.exists()) {
        return;
    }

    configFile.open(QIODevice::ReadOnly);
    QJsonDocument config = QJsonDocument::fromJson(configFile.readAll());

    configFile.close();

    m_data = config.toVariant();

    QList<QVariant> items = m_data.toList();

    for (QVariant & item : items) {
        QHash<QString,QVariant> section = item.toHash();

        if (section["name"].toString() == "outputs") {
            QList<QVariant> outputs = section["data"].toList();
            int n = 0;

            for (QVariant & output : outputs) {
                QHash<QString,QVariant> outputData = output.toHash();
                QString connector = outputData["connectorName"].toString();
                QString edid = readEdid("/sys/class/drm/card1-" + connector + "/edid");

                QCheckBox* checkBox = new QCheckBox(edid);
                checkBox->setProperty("screenIndex",n);

                connect(checkBox, &QCheckBox::checkStateChanged, [this,checkBox]()
                {
                    int index = checkBox->property("screenIndex").toInt();
                    m_enable[index] = (checkBox->checkState() == Qt::Checked);
                });

                m_screenList->addWidget(checkBox);

                checkBox->setCheckState(Qt::Checked);
                m_enable[n] = true;

                for (QString screenRule : screenDB) {
                    QRegularExpression rule(screenRule);

                    if (rule.match(edid).hasMatch()) {
                        qDebug()<<"Match "<<edid<<" with "<<screenRule;

                        checkBox->setCheckState(Qt::Unchecked);
                        m_enable[n] = false;
                    }
                }

                n++;
            }
        }
    }

}

void MainWindow::exportConfig()
{

    QList<QVariant> items = m_data.toList();

    QList<QVariant> nitems;

    for (QVariant & item : items) {
        QHash<QString,QVariant> section = item.toHash();

        if (section["name"].toString() == "setups") {
            QList<QVariant> setups = section["data"].toList();
            QList<QVariant> nsetups;

            for (QVariant & setup : setups) {
                QHash<QString,QVariant> setupData = setup.toHash();

                QList<QVariant> outputs = setupData["outputs"].toList();
                if (outputs.count() < 2) {
                    nsetups<<setupData;
                    continue;
                }

                QList<QVariant> noutputs;

                for (QVariant & output : outputs) {
                    QHash<QString,QVariant> outputData = output.toHash();
                    int index = outputData["outputIndex"].toInt();
                    //outputData["enabled"] = true;
                    qDebug()<<"index:"<<index;

                    outputData["enabled"] = m_enable[index];

                    noutputs<<outputData;
                }

                setupData["outputs"] = noutputs;
                nsetups<<setupData;
            }

            section["data"] = nsetups;
        }

        nitems<<section;
    }

    QFile configFile;
    configFile.setFileName("/tmp/kwinoutputconfig.json");

    configFile.open(QIODevice::ReadWrite);
    QJsonDocument out = QJsonDocument::fromVariant(nitems);

    configFile.write(out.toJson());

    configFile.close();

    runHelper("/tmp/kwinoutputconfig.json");
}

void MainWindow::runHelper(QString path)
{
    QProcess helper;
    QStringList args;

    args<<"/usr/libexec/lliurex-screen-disabler-helper"<<path;

    helper.start("/usr/bin/pkexec",args);
    helper.waitForFinished(3000);
}
