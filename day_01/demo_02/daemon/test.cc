#include "Daemon.hpp"

int main()
{
    Daemon(true, true);
    // daemon(0, 0);
    while (true)
    {
        sleep(1);
    }
    return 0;
}