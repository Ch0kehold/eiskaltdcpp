/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#pragma once

#include <gtk/gtk.h>
#include <dcpp/stdinc.h>
#include <dcpp/typedefs.h>
#include "entry.hh"

class UserCommandMenu : public Entry
{
public:
    UserCommandMenu(GtkWidget *userCommandMenu, int ctx);
    virtual ~UserCommandMenu() {}

    GtkWidget *getContainer() { return userCommandMenu; }
    void addHub(const std::string &hub);
    void addHub(const dcpp::StringList &hubs2);
    void addUser(const std::string &cid);
    void addFile(const std::string &cid, const std::string &name, const std::string &path,
                 const int64_t &size = 0, const std::string &tth = "");
    void cleanMenu_gui();
    void buildMenu_gui();

private:
    // GUI functions
    void createSubMenu_gui(GtkWidget *&menu, std::string &command);

    // GUI callbacks
    static void onUserCommandClick_gui(GtkMenuItem *item, gpointer data);

    // Client functions
    void sendUserCommand_client(std::string cid, std::string commandName, std::string hub, dcpp::StringMap params);

    GtkWidget *userCommandMenu;
    int ctx;
    dcpp::StringList hubs;
    struct UCParam
    {
        std::string cid;
        std::string name;
        std::string path;
        std::string tth;
        std::string type;
        int64_t size = 0;
    };
    std::vector<UCParam> ucParams;
};
