// SPDX-FileCopyrightText: 2026 Enrique M.G. <quique@necos.es>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LLX_SD_MAINWINDOW
#define LLX_SD_MAINWINDOW

#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QVariant>
#include <QMap>

namespace lliurex
{
    class MainWindow: public QMainWindow
    {
        public:

        MainWindow(QWidget* parent = nullptr);
        virtual ~MainWindow();

        private:

        void loadConfig();
        void exportConfig();

        QVariant m_data;
        QVBoxLayout* m_layout;
        QVBoxLayout* m_screenList;
        QMap<int,bool> m_enable;

    };
}

#endif
