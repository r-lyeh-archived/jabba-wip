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

bool operator==( const point &a, const point &b ) {
    return fabs(a.x - b.x) < 1e-6 && fabs(a.y - b.y) < 1e-6;
}
bool operator!=( const point &a, const point &b ) {
    return !(a == b);
}
point operator+( const point &a, const point &b ) {
    point c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    return c;
}
point operator*( const point &a, float f ) {
    point c = a;
    c.x *= f;
    c.y *= f;
    return c;
}

struct rect : public point {
    float w = -1, h = -1; 
    std::string name;
};
MEDEA_DEFINE( rect &it, ( (point&)it, it.w, it.h, it.name ) );

bool operator==( const rect &a, const rect &b ) {
    if( (point&)a != (point&)b) return false;
    return fabs(a.w - b.w) < 1e-6 && fabs(a.h - b.h) < 1e-6;
}
bool operator!=( const rect &a, const rect &b ) {
    return !(a == b);
}
rect operator+( const rect &a, const rect &b ) {
    rect c;
    ((point &)c) = (point)a + (point)b;
    c.w = a.w + b.w;
    c.h = a.h + b.h;
    c.name = a.name;
    return c;
}
rect operator*( const rect &a, float f ) {
    rect c = a;
    ((point &)c) = (point)a * f;
    c.w *= f;
    c.h *= f;
    return c;
}

struct layout : public rect {
    mutable std::map< char, rect > children; // per name (@todo, per depth)

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

std::string dump( const layout &lyt_, int w = 42, int h = 10 ) {
    auto lyt = rescale( lyt_, w, h );

    // fix
    for( auto &pair : lyt.children ) {
        auto &name = pair.first;
        auto &rect = pair.second;
    }

    // fill
    wire::strings screen;
    for( int y = 0; y < h; ++y ) {
        screen.push_back( wire::string( w, '.' ) );
    }

    // invert depth
    std::vector< std::pair< char, rect > > reverse( lyt.children.rbegin(), lyt.children.rend() );

    // render
    for( auto &pair : reverse ) {
        auto &name = pair.first;
        auto &rect = pair.second;

        for( auto y = rect.y; y < rect.y + rect.h; ++y ) 
        for( auto x = rect.x; x < rect.x + rect.w; ++x ) 
        screen[ y ][ x ] = name;
    }    

    return screen.str("\1\n") + debug(lyt);
}

#include "deps/sand/sand.hpp"
#include "deps/sand/sand.cpp"
#include "deps/tween/tween.hpp"
#include "deps/rlutil/rlutil.h"
#include "deps/rand/rand.hpp"

template<typename T>
T interpolate( float dt01, const T &src, const T &dst ) {
    return src * (1-dt01) + dst * dt01;
}

template<typename T>
T interpolate( float dt01, const T &src, const T &dst, int ease ) {
    return interpolate( tween::ease( ease, dt01 ), src, dst );
}

template<typename T>
struct tweener {
    int ease;
    const T *src, *dst;
    T *store;
    float delay, t0, t1;

    tweener( int ease, float delay, float t1, const T &src, const T &dst, T &store ) : ease(ease),src(&src),dst(&dst),store(&store),delay(delay),t0(-delay),t1(t1)
    {}

    bool operator()( float dt ) {
        t0 += dt;
        if(t0 < 0) {
            return false;
        }
        if( t0 > t1 ) {
            t0 = t1;
        }
        float dt01 = t0 / t1;
        auto before = *store;
        auto after = ( *store = interpolate( dt01, *src, *dst, ease ) );
        bool has_finished = dt01 >= 1 && before == after;
        return has_finished;
    }

    void reset() {
        t0 = -delay;
        *store = *src;
    }
    void swap() {
        float left = t1 - t0;
        t0 = left;

        const T *swap = src;
        src = dst;
        dst = swap;            
    }
};

using tweeners = std::vector< tweener<rect> >;

bool run( tweeners &tws, float dt ) {
    unsigned finished = 0;
    for( auto &tw : tws ) {
        finished += tw(dt);
    }
    return finished != tws.size();
}
void swap( tweeners &tws ) {
    for( auto &tw : tws ) {
        tw.swap();
    }
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

    hidecursor();
    cls();

    auto copy = lyt;

    int ease = tween::SWING; // rand() % tween::TOTAL; //

    std::vector< tweener<rect> > tws { 
        { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['a'], copy['b'], lyt['a'] },
        { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['b'], copy['c'], lyt['b'] },
        { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['c'], copy['d'], lyt['c'] },
        { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['d'], copy['e'], lyt['d'] },
        { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['e'], copy['f'], lyt['e'] },
        { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['f'], copy['g'], lyt['f'] },
        { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['g'], copy['h'], lyt['g'] },
        { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['h'], copy['a'], lyt['h'] },
    };

    for(;;) {
        sand::sleep(1/60.f);

        if( !run(tws, 1/60.f) ) {
//          break;
        }

        if( kbhit() ) {
			anykey();
            swap(tws);
        }
        
        locate(1, 1);
        printf("%s", dump(lyt).c_str());
        fflush(stdout);
    }

    showcursor();
}

