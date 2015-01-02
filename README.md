jabba
=====

jabba the hud. yet another generic hud/ui engine (WIP WIP)

## sample
```c++
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

C:\prj\jabba>layout
dimensions: 1280x720
dimensions of a: 47x53 @(0,0)
dimensions of b: 47x53 @(1232,0)
dimensions of c: 47x53 @(379,144)
dimensions of d: 47x53 @(900,144)
dimensions of e: 47x53 @(379,288)
dimensions of f: 142x106 @(900,288)
dimensions of g: 47x53 @(0,576)
dimensions of h: 616x53 @(663,576)
```

