#pragma once
#include <string>

/*
struct rect {
    unsigned w, h;
    char patch[9];
}; */

struct textbox {
    unsigned w, h;
    //rect r;
    std::string body;
};

struct window {
    std::string title;
    bool shadow;
    textbox tx;
};

#include "deps/wire/wire.hpp"

static inline
std::string segment( unsigned chars, char fill ) {
    return std::string( chars, fill );
}

static inline
std::string left( const std::string &text, unsigned width, char fill ) {
    if( text.size() > width ) return text.substr(0, width - 3) + "...";
    unsigned tab = width - text.size();
    return text + segment( tab, fill );
}

static inline
std::string centered( const std::string &text, unsigned width, char fill ) {
    if( text.size() > width ) return text.substr(0, width - 3) + "...";
    unsigned tab = width - text.size();
    return segment( tab/2, fill ) + text + segment( tab/2 + tab%2, fill );
}

static inline
std::string box( const std::string &text, int width = 80 - 4 ) {
    std::string line = segment( width, '-' ), blank = segment( width, ' ' ), xxx = segment( width, 'X' );
    return wire::string(".\1.\n|\2|X\n|\3|X\n|\4|X\n'\5'X\n \6XX\n", line, blank, centered(text, width, ' '), blank, line, xxx );
}

template<typename anything>
static inline
std::ostream &print( const anything &t, std::ostream &os = std::clog ) {
    return os << t, os;
}
template<typename anything>
static inline
std::ostream &println( const anything &t, std::ostream &os = std::clog ) {
    return os << t << std::endl, os;
}

static inline
std::string render( const window &w ) {
    const textbox &tx = w.tx;

    if( tx.w < 2 || tx.h < 2 ) {
        return std::string();
    }

    auto separator = [&]( char ch, char sp, char ch2 ) {
        return wire::string("\1\2\3", ch, wire::string(sp, tx.w-2), ch2 );
    };

    wire::string out;
    wire::strings lines = wire::string( tx.body ).tokenize("\r\n");

    for( auto &ln : lines ) {
        ln = wire::string("\1\2\1", '|', left( ln, tx.w - 2, ' ' ) );
    }

    // title
    if( w.title.size() ) {
        lines.push_front( separator('|','-','|') );
        lines.push_front( wire::string("\1\2\1", '|', centered(w.title, tx.w-2, ' ') ));
    }

    // header
    lines.push_front( separator('.', '-', '.') );

    // footer
    lines.push_back( separator('`', '-', '\'') );

    if( w.shadow ) {
        for( auto &l : lines ) {
            l = l + 'X';
        }
        lines.front().back() = ' ';
        lines.push_back( " " + segment(tx.w, 'X') );
    }

    return lines.str("\1\n");    
}
