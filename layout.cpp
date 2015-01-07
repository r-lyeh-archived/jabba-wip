// ascii layout parser
// - rlyeh

#include "deps/hyde/hyde.hpp"
#include "deps/hyde/hyde.cpp"

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
bool inside( const rect &r, float x, float y ) {
    if( x < r.x ) return false;
    if( y < r.y ) return false;
    if( x > r.x + r.w ) return false;
    if( y > r.y + r.h ) return false;
    return true;
}
bool inside( const rect &r, const point &p ) {
    return inside( r, p.x, p.y );
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

#include <functional>
template<typename USERDEF>
using events = std::map< std::string, std::function<bool(const rect &r, const USERDEF &inputs)> >;
using callbacks = std::vector< std::function<void()> >;

struct cursor {
    int x, y, w, h;
};
cursor refresh() {
    CONSOLE_SCREEN_BUFFER_INFO a;
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hStdOut,&a);

    static SHORT CX = 0, CY = 0;

    CONSOLE_FONT_INFO ci;
    GetCurrentConsoleFont(hStdOut,FALSE,&ci);

    POINT p;
    if (GetCursorPos(&p)) {
        if (ScreenToClient(GetConsoleWindow(), &p)) {
            CX = p.x / ci.dwFontSize.X;
            CY = p.y / ci.dwFontSize.Y;
        }
    }

    return { CX, CY, a.srWindow.Right - a.srWindow.Left, a.srWindow.Bottom - a.srWindow.Top };
}

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
    std::string horizontal = $QUOTE(
        a.........................b
        ........c..........d.......
        ........e..........fff.....
        ...................fff.....
        g.............hhhhhhhhhhhhh
    ),
    vertical = $QUOTE(
        a...b.c.d...e
        .............
        .............
        .............
        ...........ff
        g........hhhh
    );

    bool using_horizontal = true;
    auto lyt = parse(horizontal, 1280, 720);
    std::cout << debug( lyt ) << std::endl;

    using namespace rlutil;

    hidecursor();
    cls();

    auto copy = lyt;
    int ease = tween::SWING; // rand() % tween::TOTAL; //

    std::vector< tweener<rect> > tws;

    struct input {
        int mousex, mousey;
    };

    std::string logger;
    events<input> evs = { 
        { "hover", [&]( const rect &r, const input &inp ) {
            if( inside( r, inp.mousex, inp.mousey ) ) {
                logger += r.name + ',';
            }
            return true;
        } }
    };

    auto setup = [&]() {
        if( using_horizontal ) {
            lyt = parse(horizontal, 1280, 720);
        } else {
            lyt = parse(vertical, 720, 1280);            
        }
        copy = lyt;
        tws = std::vector< tweener<rect> > { 
            { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['a'], copy['b'], lyt['a'] },
            { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['b'], copy['c'], lyt['b'] },
            { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['c'], copy['d'], lyt['c'] },
            { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['d'], copy['e'], lyt['d'] },
            { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['e'], copy['f'], lyt['e'] },
            { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['f'], copy['g'], lyt['f'] },
            { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['g'], copy['h'], lyt['g'] },
            { ease, rand2(0.125f,0.5f), rand2(0.250f,0.500f), copy['h'], copy['a'], lyt['h'] } 
        };        
    };

    setup();

    hyde::windows::mouse mouse(0,1);

    for(;;) {
        sand::sleep(1/60.f);
        mouse.update();

        if( !run(tws, 1/60.f) ) {
//          break;
        }

        if( kbhit() ) {
            char ch = getch();
            swap(tws);

            if( ch == ' ' ) {
                using_horizontal ^= true;
                setup();
                cls();
            }
        }
        
        locate(1, 1);
        if( using_horizontal ) {
            printf("%s", dump(lyt, 42, 10).c_str());
        } else {
            printf("%s", dump(lyt, 10*2, 42/2).c_str());
        }

        auto arrow = refresh();
        // console chars to virtual desktop
        if( using_horizontal ) {
            arrow.w = 42;
            arrow.h = 10;
            arrow.x = ++arrow.x * 1280 / float(arrow.w);
            arrow.y =   arrow.y *  720 / float(arrow.h);
        } else {
            arrow.w = 10*2;
            arrow.h = 42/2;
            arrow.x =    arrow.x *  720 / float(arrow.w);
            arrow.y =  ++arrow.y * 1280 / float(arrow.h);
        }

        std::cout << arrow.x << ',' << arrow.y << ',' << arrow.w << ',' << arrow.h << " vs " << lyt['e'].x << ',' << lyt['e'].y << ' ' << std::endl;

        mouse.clipped.set( mouse.right.hold() );
        std::cout
            << "mouse[0]="
            << "left["    << (mouse.left.hold()    ? "x]" : " ]")
            << "middle["  << (mouse.middle.hold()  ? "x]" : " ]")
            << "right["   << (mouse.right.hold()   ? "x]" : " ]")
            << "hover["   << (mouse.hover.hold()   ? "x]" : " ]")
            << "hidden["  << (mouse.hidden.hold()  ? "x]" : " ]")
            << "clipped[" << (mouse.clipped.hold() ? "x]" : " ]")
            << " @global(" << mouse.global.newest().x << ',' << mouse.global.newest().y << ")"
            << " @local("  << mouse.local.newest().x  << ',' << mouse.local.newest().y  << ")"
            << " @wheel("  << mouse.wheel.newest().y << ")"
            << "               \n";

        logger.clear();
        input inp { arrow.x, arrow.y };
        for( auto &rect : lyt.children ) {
            for( auto &evpair : evs ) {
                auto &ev = evpair.second;
                if( ev ) {
                    ev( rect.second, inp );
                }
            }
        }
        if( logger.empty() ) logger = wire::string(40, ' ');
        printf("%s\n", logger.c_str());

        fflush(stdout);
    }

    showcursor();
}

