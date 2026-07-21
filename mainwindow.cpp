// SPDX-FileCopyrightText: 2026 Enrique M.G. <quique@necos.es>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.hpp"

extern "C" {
    #include <libdisplay-info/info.h>
    #include <libdisplay-info/edid.h>
}

#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QIcon>
#include <QJsonDocument>
#include <QFile>
#include <QVariant>
#include <QDir>

#include <fstream>

using namespace lliurex;
using namespace std;

QString readEdid(QString path)
{
    QString value;

    qDebug()<<"reading edid "<<path;
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

    qDebug()<<"Edid size "<<buffer.size();

    struct di_info* info;

    info = di_info_parse_edid((void *)buffer.data(),buffer.size());
    value = di_info_get_model(info);
    qDebug()<<"model:"<<value;
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

    btn = new QPushButton(this);
    btn->setIcon(QIcon::fromTheme("document-new"));
    connect(btn, &QPushButton::clicked, [this]()
    {
        qDebug()<<"loading config...";
        loadConfig();
    });
    toolbar->addWidget(btn);

    btn = new QPushButton(this);
    btn->setIcon(QIcon::fromTheme("document-save"));
    connect(btn, &QPushButton::clicked, [this]()
    {
        qDebug()<<"saving config...";
        exportConfig();
    });
    toolbar->addWidget(btn);


    toolbar->addStretch();

    setWindowTitle("Lliurex Screen Disabler");
    resize(800, 600);

    m_layout->addLayout(toolbar);
    m_layout->addLayout(m_screenList);
    m_layout->addStretch();

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
                qDebug()<<edid;
                qDebug()<<n;

                QCheckBox* checkBox = new QCheckBox(edid);
                checkBox->setProperty("screenIndex",n);

                connect(checkBox, &QCheckBox::checkStateChanged, [this,checkBox]()
                {
                    qDebug()<<"click";

                    int index = checkBox->property("screenIndex").toInt();
                    m_enable[index] = (checkBox->checkState() == Qt::Checked);

                });

                m_screenList->addWidget(checkBox);

                if (edid == "L01N8A") {
                    checkBox->setCheckState(Qt::Unchecked);
                    m_enable[n] = false;
                }
                else {
                    checkBox->setCheckState(Qt::Checked);
                    m_enable[n] = true;
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
    configFile.setFileName("kwinoutputconfig.json");

    configFile.open(QIODevice::ReadWrite);
    QJsonDocument out = QJsonDocument::fromVariant(nitems);

    configFile.write(out.toJson());

    configFile.close();
}
