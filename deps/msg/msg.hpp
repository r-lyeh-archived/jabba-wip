// A lightweight ring based application framework. Written in C++11
// - rlyeh. BOOST licensed

// some theory:
// a ring is made of independant systems
// rings depend on inner rings
// systems communicate through messages, which are captured, ignored and broadcasted by callbacks

// pros:
// structured way to init/shutdown/restart rings
// flexible workflow
// no need for singletons
// unmanaged interdependencies

// cons:
// slower than direct calls to singletons

#pragma once
#include <deque>
#include <vector>

#ifndef FRODO_CUSTOM_MESSAGE
// api
struct message {
    int code;
    std::string text;

#if 0
    // vs2014 unrestricted unions
    union U {
        std::string text;

        U() {
            new(&text) std::string();
        }
    } u;
#endif

    int id;
    int timestamp;
};
#endif

class system2 {
public:
    system2()
    {}
    virtual ~system2() {}
    virtual int handle( message &msg ) = 0;
};

using ring = std::vector< system2* >;
using base = std::vector< ring >;

class framework : public base {
public:
    framework()
    {}
    framework( std::initializer_list<ring> t) : base(t)
    {}
    void push_ring( const ring &r ) {
        this->push_back( r );
    }
    void push_ring( system2 *r ) {
        push_ring( ring{ r } );
    }
    int message( const message &msg_ ) {
        static int id = 0;
        ::message msg = msg_;
        msg.id = ++id;
        bool keep = true;
        if( keep ) for( auto ring = this->rbegin(), end = this->rend(); ring != end; ++ring ) {
            if( keep ) for( auto &system2 : *ring ) {
                if( (keep = system2->handle(msg)) ) continue;
            }
        }
        printf(" [%d] %s\n", msg.id, keep ? ":)" : ":(" );
        return keep;
    }
    int operator()( const ::message &msg ) {
        return this->message( msg );
    }
};

extern framework app;
