// jabba, the hud engine. so simple it hurts. boost licensed.
// - rlyeh ~~ listening to witchrider / black

// workflow
// - define identifiers, positions and volumes (~rects)
// - define properties and events per rect
// - assign callbacks
// - while( game ) { fullfil tree with events; render tree as HUD; }
//

// example:
// - rect dialog = (640,480,@bottom)
// - dialog["visible"] = false;
// - dialog["@click"] = []{ dialog["visible"] ^= true; }
// - render();

#pragma once
#include "deps/medea/medea.hpp"     // json
#include "deps/wire/wire.hpp"       // strings
#include "deps/sand/sand.hpp"       // easings
#include "deps/sand/sand.cpp"       // easings
#include "deps/signals/signals.hpp" // signals

#include <map>
#include <functional>

namespace jabba {
    // core types
    using string = wire::string;               // generic type that eats anything: booleans, integers, floats, doubles, lambdas, C strings, std::strings, etc
    using keyval = std::map< string, string >; // app-defined properties

    // api types
    using widget = std::map< string, string >; // app-defined (non/) interactive element that can be displayed, hidden, etc
    using screen = std::map< string, widget >; // collection of widgets;
    using stream = std::map< string, screen >; // game flow of screens; 

    // callbacks
    struct event;
    using callback = std::function< void(event *) >;

    struct event {
        callback fn;
        keyval in, out;
        bool operator()() {
            if(fn) fn(this);
            return true;
        }
    };

    // engine
    struct hud {
        stream flow;
        std::map< int, event > on;
        std::map< int, event > off;

        template<typename T>
        bool walk( const T &t ) {
            for( auto &screen : flow ) {
                for( auto &widget : screen.second ) {
                    if( !on(t, widget) ) {
                        return false;
                    }
                }
            }
            return true;
        }

        bool load( const std::string &src ) {
            return true;
        }
        bool save( std::string &dst ) const {
            return true;
        }
    };
}


// example

#include <iostream>

enum {
    setup,
    load,
    save,
    quit,

    update,
    render,
};

int main() {
    jabba::hud engine;

    engine.on[setup] = {
        []( jabba::event *self ) {
        std::cout << "ignition" << std::endl;
    }}; 
    engine.off[setup] = {
        []( jabba::event *self ) {
        std::cout << "bye bye" << std::endl;
    }};

    engine.on[load] = {
        []( jabba::event *self ) {
        std::cout << "loading..." << std::endl;
    }};

    engine.on[setup]();
    engine.on[load]();

    engine.on[update]();
    engine.on[render]();

    engine.off[setup]();
}
