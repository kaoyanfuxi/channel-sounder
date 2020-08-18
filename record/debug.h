/*

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef CHANNELSOUNDER_DEBUG_H
#define CHANNELSOUNDER_DEBUG_H

#include <iostream>
#include <cassert>
#include <chrono>

//#define NDEBUG_CST
#ifdef NDEBUG_CST
#define DBG_RB(x)
#else
#define DBG_RB(x)     x
#endif

namespace channelsounder
{
// we use these variables to debug the multithreaded channel measurement
struct stats{
    std::chrono::high_resolution_clock::time_point tstart;
    std::chrono::high_resolution_clock::time_point tend;
    unsigned long long n_samples_total;                         // total number of received samples per channel
    unsigned long long n_full;                                  // how often did we fill one of the two buffers/fifos?
    unsigned long long n_worker_wait;                           // how often did we enter the wait state in the worker?
    unsigned long long n_worker_executed;                       // how often did the worker execute his task after notify_all()?
    unsigned long long n_worker_not_done;                       // how often was a buffer full but the worker thread was not done processing the old thread?
    
    void reset(){
        tstart = std::chrono::high_resolution_clock::now();
        tend = std::chrono::high_resolution_clock::now();
        n_samples_total = 0;
        n_full = 0;
        n_worker_wait = 0;
        n_worker_executed = 0;
        n_worker_not_done = 0;
    };
    
    void print_data(std::string source){
        tend = std::chrono::high_resolution_clock::now();
        double exec_time = (double) std::chrono::duration_cast<std::chrono::seconds>(tend - tstart).count();
        double data_rate = ((double) n_samples_total)/exec_time;    
    
        std::cout << "--------------------------" << std::endl;
        std::cout << source << std::endl;
        std::cout << "Execution time in sec: " << exec_time << std::endl;
        std::cout << "Data rate in MS/sec: " << data_rate/1e6 << std::endl;
        std::cout << "local_stats.n_samples_total: " << n_samples_total << std::endl;
        std::cout << "local_stats.n_full: " << n_full << std::endl;
        std::cout << "local_stats.n_worker_wait: " << n_worker_wait << std::endl;
        std::cout << "local_stats.n_worker_executed: " << n_worker_executed << std::endl;
        std::cout << "local_stats.n_worker_not_done: " << n_worker_not_done << std::endl;
        std::cout << "--------------------------" << std::endl;    
    }
};
}
#endif
