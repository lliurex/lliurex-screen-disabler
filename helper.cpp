// SPDX-FileCopyrightText: 2026 Enrique M.G. <quique@necos.es>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <string>
#include <iostream>
#include <filesystem>

using namespace std;

int main(int argc,char* argv[])
{
    const std::filesystem::path sddm_home = "/var/lib/sddm/";
    const std::filesystem::path destination = sddm_home / ".config";

    if (argc < 2) {
        //silent exit
        return 0;
    }

    try {

        std::filesystem::path source = argv[1];

        if (!std::filesystem::exists(sddm_home)) {
            cerr<<"error: sddm home not found"<<endl;
            return 1;
        }

        if (!std::filesystem::exists(destination)) {
            std::filesystem::create_directory(destination);
            //TODO: fix ownership
        }

        std::filesystem::copy(source,destination);

    }
    catch(std::exception& e) {
        cerr<<e.what()<<endl;

        return 1;
    }

    return 0;
}
