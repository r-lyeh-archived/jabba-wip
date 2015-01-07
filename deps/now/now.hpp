#include <chrono>
#include <thread>
 
auto epoch = std::chrono::steady_clock::now(); 
double now() {
    return std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - epoch ).count() / 1000.0;
};
void sleep( double secs ) {
    std::chrono::microseconds duration( (int)(secs * 1000000) );
    std::this_thread::sleep_for( duration );    
}

class dt {
	auto now = ::now();
	dt() : no
}
