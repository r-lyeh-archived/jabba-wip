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
    mutable std::map< char, rect > children;

    const rect &operator[]( char x ) const {
        return children[ x ];
    }
    rect &operator[]( char x ) {
        return children[ x ];
    }
};
MEDEA_DEFINE( layout &it, ( (rect&)it, it.children ) );

layout rescale( const layout &lyt_, float w, float h ) {
    auto lyt = lyt_;
    w = w / lyt.w;
    h = h / lyt.h;
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

    lyt = rescale( lyt, resw, resh );

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

std::string dump( const layout &lyt_, int w = 27, int h = 5 ) {
    auto lyt = rescale( lyt_, w, h );

    for( auto &pair : lyt.children ) {
        auto &name = pair.first;
        auto &rect = pair.second;

        if( rect.w == 0 ) rect.w++;
        if( rect.h == 0 ) rect.h++;
    }

    wire::strings screen;
    for( int y = 0; y < h; ++y ) {
        screen.push_back( wire::string( w, '.' ) );
    }

    for( auto &pair : lyt.children ) {
        auto &name = pair.first;
        auto &rect = pair.second;

        for( auto y = rect.y; y < rect.y + rect.h; ++y ) 
        for( auto x = rect.x; x < rect.x + rect.w; ++x ) 
        screen[ y ][ x ] = name;
    }    

    return screen.str("\1\n");
}

#include "deps/sand/sand.hpp"
#include "deps/sand/sand.cpp"
#include "deps/tween/tween.hpp"
#include "deps/rlutil/rlutil.h"

#include <thread>
std::thread morph( layout &lyt, char from, char to, float secs, int ease ) {
    return std::thread( [&]{
        auto &cur = lyt[from], src = cur;
        auto &dst = lyt[to];
        sand::chrono lp( secs );
        while( lp.s() < 1 ) {
            float dt01 = tween::bouncein( lp.s() ); //tween::ease( lp.s(), ease );
            cur.x = src.x * (1-dt01) + dst.x * dt01;
            cur.y = src.y * (1-dt01) + dst.y * dt01;
            cur.w = src.w * (1-dt01) + dst.w * dt01;
            cur.h = src.h * (1-dt01) + dst.h * dt01;
            //sand::sleep(1/60.f);            
        }
    });
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

    auto lyt = parse(test, 1280, 720);
    std::cout << debug( lyt ) << std::endl;

    using namespace rlutil;

    float from = lyt['b'].x;
    float to = lyt['a'].x;

    hidecursor();
    cls();

    auto th = morph(lyt,'h','f',3.5,tween::BOUNCEIN);

    sand::chrono lp( 3.5 );
    while( lp.s() < 1 ) {
        locate(1, 1);
        printf("%s", dump(lyt).c_str());
        fflush(stdout);
    }

    th.join();
}

