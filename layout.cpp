// ascii layout parser
// - rlyeh

#include <string>
#include <map>
#include "deps/wire/wire.hpp"
#include "deps/medea/medea.hpp"

struct point {
    float x = unsigned(~0u)/2, y = unsigned(~0u)/2;
};
MEDEA_DEFINE( point &it, (it.x, it.y) );

struct rect : public point {
    float w = -1, h = -1; 
    std::string name;
};
MEDEA_DEFINE( rect &it, ( (point&)it, it.w, it.h, it.name ) );

struct layout : public rect {
    std::map< char, rect > children;
};
MEDEA_DEFINE( layout &it, ( (rect&)it, it.children ) );

layout rescale( const layout &lyt_, float w, float h ) {
    auto lyt = lyt_;
    lyt.x *= w;
    lyt.y *= h;
    lyt.w *= w;
    lyt.h *= h;
    for( auto &pair : lyt.children ) {
        auto &rect = pair.second;
        rect.x *= w;
        rect.y *= h;
        rect.w *= w;
        rect.h *= h;
    }   
    return lyt;
}

layout parse( const std::string &text, float resw, float resh, float truetype_aspect_ratio = 0.5 /* usually 1:1 or 1:2 */ ) {
    auto lines = wire::string( text ).tokenize("\t\f\n\v\r ");

    layout lyt;
    lyt.w = lines[0].size();
    lyt.h = lines.size();

    // ascii dimensions

    for( auto y = 0u; y < lines.size(); ++y ) {
        for( auto x = 0u; x < lines[y].size(); ++x ) {
            const char chr = (lines[y][x]);
            if( chr == '.' ) continue;

            auto &rect = lyt.children[chr];
            rect.name = chr;
            if( x < rect.x ) rect.x = x;
            if( y < rect.y ) rect.y = y;
            if( x > rect.w ) rect.w = x;
            if( y > rect.h ) rect.h = y;
        }
    }

    for( auto &pair : lyt.children ) {
        auto &rect = pair.second;
        rect.w -= rect.x - 1;
        rect.h -= rect.y - 1;
    }

    // what is the aspect ratio you want to describe?
    float ar = lyt.w / lyt.h * truetype_aspect_ratio;

    lyt = rescale( lyt, resw / lyt.w, resh / lyt.h );

    // try to compensate text-editors font ratio (unless you use a 1:1 font)
    for( auto &pair : lyt.children ) {
        auto &rect = pair.second;
        rect.h /= ar;
    }
    return lyt;
}

std::string debug( const layout &lyt ) {
    wire::string str("dimensions: \1x\2\n", lyt.w, lyt.h);
    for( auto &pair : lyt.children ) {
        auto &rect = pair.second;
        str += wire::string("dimensions of \1: \2x\3 @(\4,\5)\n", rect.name, int(rect.w), int(rect.h), int(rect.x), int(rect.y) );
    }
    return str;
    return medea::to_json( lyt );
}


int main() {
#   define $QUOTE(...) #__VA_ARGS__
    std::string test = $QUOTE(
        a.........................b
        ........c..........d.......
        ........e..........fff.....
        ...................fff.....
        g.............hhhhhhhhhhhhh
    ); 

    std::cout << debug( parse(test, 1280, 720) ) << std::endl;
}

