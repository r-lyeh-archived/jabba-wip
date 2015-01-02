// sample app

#include <stdlib.h>
#include <iostream>
#include <string>

#include "msg.hpp"

// define messages for this app

enum messages {
    PRNT = 'PRNT',
    LOG = 'LOG '
};

class console : public system2 {
public:
    console()
    {}
    int handle( message &msg ) {
        if( msg.code == LOG ) {
            std::cout << "console: " << msg.text << std::endl;
            return 1;
        }
        return 0;
    }
};

class ui : public system2 {
public:
    ui()
    {}
    int handle( message &msg ) {
        if( msg.code == PRNT ) {
            std::cout << "ui: " << msg.text << std::endl;
            msg.code = LOG; // log this as well
            msg.text += " (captured!)";
            return 1;
        }
        return 1;
    }
};

int main() {
    framework app = {
        // ring level 0
        { new console },
        // ring level 1
        { new ui }
    };

    app.message( { PRNT, "this is a dialog" } );
    app.message( { LOG, "this is a sample" } );
    app.message( { 100, "this message will be lost"} );

    return 0;
}



