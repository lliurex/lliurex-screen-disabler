// SPDX-FileCopyrightText: 2026 Enrique M.G. <quique@necos.es>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.hpp"

#include <QApplication>

#include <iostream>

using namespace lliurex;
using namespace std;

int main(int argc,char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
