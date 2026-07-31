// SPDX-FileCopyrightText: 2026 Enrique M.G. <quique@necos.es>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

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

    struct passwd* pwd = getpwnam("sddm");

    if (!pwd) {
        cerr<<"Failed to retrieve sddm user"<<endl;

        return 1;
    }

    try {

        std::filesystem::path source = argv[1];

        if (!std::filesystem::exists(sddm_home)) {
            cerr<<"error: sddm home not found"<<endl;
            return 1;
        }

        if (!std::filesystem::exists(destination)) {
            std::filesystem::create_directory(destination);
            chown(destination.c_str(), pwd->pw_uid, pwd->pw_gid);
        }

        std::filesystem::copy(source,destination);

        std::filesystem::path filepath = destination / source.filename();
        chown(filepath.c_str(), pwd->pw_uid, pwd->pw_gid);

    }
    catch(std::exception& e) {
        cerr<<e.what()<<endl;

        return 1;
    }

    return 0;
}
