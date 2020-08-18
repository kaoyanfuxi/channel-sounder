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

#include <iostream>
#include <fstream>
#include <boost/thread/thread.hpp>
#include <iomanip>

#include "debug.h"
#include "config.h"
#include "fifo_ch_measurement.h"

#define CH_MEASUREMENT_PER_SEC              1000
#define CH_MEASUREMENT_LENGTH_IN_SAMPLES    500
#define CH_MEASUREMENT_SAVE_PERIOD_SEC      10
#define CH_MEASUREMENT_SAVE_PERIOD          CH_MEASUREMENT_PER_SEC*CH_MEASUREMENT_SAVE_PERIOD_SEC

namespace channelsounder
{
static size_t n_channels;                   // number of channels/antennas, set in init function
static size_t n_bytes_per_item;             // size of complex sample
static unsigned int samp_rate;              // S/s
static unsigned int n_samples_per_period;   // number of complex samples between two channel measurements
    
enum buffer_enum{
    NO_BUFFER = -1,
    BUFFER0 = 0,
    BUFFER1 = 1
};
static buffer_enum buffer2write;
static buffer_enum buffer2process;

// collecting samples in a state machine
enum buffer_enum_state{
    COLLECT_CHANNEL_MEASUREMENT,
    WAIT_FOR_NEW_MEASUREMENT
};
static buffer_enum_state d_STATE;
static unsigned int n_state_0;                      // state counter
static unsigned int n_state_1;                      // "
static unsigned long long n_measurement_counter;    // counts to CH_MEASUREMENT_SAVE_PERIOD, actual measurements per file
static unsigned long long n_measurement_saved;      // counts files
    
// columns: number of rx channels (antennas)
// rows: container for samples
static std::vector<std::vector<char>> buffs0;
static std::vector<std::vector<char>> buffs1;

static boost::mutex m_mutex;
static boost::condition_variable m_condition;

static struct stats local_stats;
    
static void print_data_init();
    
int init_fifo_ch_measurement(const size_t n_channels_arg, const size_t n_bytes_per_item_arg, const unsigned int samp_rate_arg){
    n_channels = n_channels_arg;
    n_bytes_per_item = n_bytes_per_item_arg;
    samp_rate = samp_rate_arg;
    n_samples_per_period = samp_rate/CH_MEASUREMENT_PER_SEC;
    
    buffer2write = BUFFER0;
    buffer2process = NO_BUFFER;
    
    // initialize buffers
    std::vector<char> buff_template(CH_MEASUREMENT_SAVE_PERIOD * CH_MEASUREMENT_LENGTH_IN_SAMPLES * n_bytes_per_item);
    for (size_t ch = 0; ch < n_channels; ch++){
        buffs0.push_back(buff_template);
        buffs1.push_back(buff_template);
    }
    
    d_STATE = COLLECT_CHANNEL_MEASUREMENT;

    n_state_0 = 0;
    n_state_1 = 0;
    n_measurement_counter = 0;
    n_measurement_saved = 0;

    print_data_init();
    local_stats.reset();
    
    return 1;
}

void feed_new_ch_measurement(const std::vector<std::vector<char>> &buffs01, const unsigned long long n_new_samples){
    DBG_RB(local_stats.n_samples_total += n_new_samples;)
    unsigned int n_consumed_samples = 0;

    while(n_consumed_samples < n_new_samples)
    {
        switch(d_STATE)
        {
            case COLLECT_CHANNEL_MEASUREMENT:
            {
                unsigned int n_residual_samples = n_new_samples - n_consumed_samples;
                unsigned int n_samples_until_measurement_complete = CH_MEASUREMENT_LENGTH_IN_SAMPLES - n_state_1;
                unsigned int n_samples_usable = std::min(n_samples_until_measurement_complete, n_residual_samples);
                
                // save binary data of this measurement
                unsigned int offest = (n_measurement_counter * CH_MEASUREMENT_LENGTH_IN_SAMPLES + n_state_1) * n_bytes_per_item;
                for(size_t ch = 0; ch < n_channels; ch++){
                    if(buffer2write == BUFFER0){
                        auto destination = buffs0[ch].begin() + offest;
                        auto source = buffs01[ch].begin() + n_consumed_samples*n_bytes_per_item;
                        std::copy_n(source, n_samples_usable*n_bytes_per_item, destination);
                    }
                    else if(buffer2write == BUFFER1){
                        auto destination = buffs1[ch].begin() + offest;
                        auto source = buffs01[ch].begin() + n_consumed_samples*n_bytes_per_item;
                        std::copy_n(source, n_samples_usable*n_bytes_per_item, destination);
                    }
                }

                n_state_1 += n_samples_usable;
                n_consumed_samples += n_samples_usable;

                // if this condition is met, we know that the measurement is complete
                if(n_state_1 == CH_MEASUREMENT_LENGTH_IN_SAMPLES){
                    d_STATE = WAIT_FOR_NEW_MEASUREMENT;
                    n_state_0 = 0;
                    n_state_1 = 0;
                    
                    n_measurement_counter++;

                    // trigger worker thread
                    if (n_measurement_counter == CH_MEASUREMENT_SAVE_PERIOD){
                        DBG_RB(local_stats.n_full++;)
                        n_measurement_counter = 0;
                        {
                            boost::mutex::scoped_lock lock(m_mutex, boost::try_to_lock);

                            // if we were able to lock the mutex, processing thread must be in waiting state, so we prepare processing and then notify worker thread
                            if(lock){
                                // swap buffers
                                if(buffer2write == BUFFER0){
                                    buffer2write = BUFFER1;
                                    buffer2process = BUFFER0;
                                }
                                else{
                                    buffer2write = BUFFER0;
                                    buffer2process = BUFFER1;
                                }
                            }
                            // if we were unable to lock the mutex, we write data into the same buffer again, therefore losing samples
                            else{
                                DBG_RB(local_stats.n_worker_not_done++;)
                                buffer2process = NO_BUFFER;                
                            }
                        }
                        m_condition.notify_all();
                    }
                }
            }
            case WAIT_FOR_NEW_MEASUREMENT:
            {
                unsigned int n_residual_samples = n_new_samples - n_consumed_samples;
                unsigned int n_samples_until_new_measurement = (n_samples_per_period - CH_MEASUREMENT_LENGTH_IN_SAMPLES) - n_state_0;
                unsigned int n_samples_skippable = std::min(n_samples_until_new_measurement, n_residual_samples);

                n_state_0 += n_samples_skippable;
                n_consumed_samples += n_samples_skippable;

                // if this condition is met, we know that a measurement is starting in this buffs01
                if(n_samples_until_new_measurement < n_residual_samples){
                    d_STATE = COLLECT_CHANNEL_MEASUREMENT;
                    n_state_0 = 0;
                    n_state_1 = 0;
                }
            }                
        }
    }
}
   
void send_save_ch_measurements(std::atomic<bool>& burst_timer_elapsed){
    
    while(1){
            boost::mutex::scoped_lock lock(m_mutex);
        
            while(buffer2process == NO_BUFFER){
                DBG_RB(local_stats.n_worker_wait++;)
                                    
                // from time to time we check if "burst_timer_elapsed" was set to true
                m_condition.wait_for(lock, boost::chrono::milliseconds(1000));
                
                // is set in main thread to stop execution
                if(burst_timer_elapsed == true)
                    return;
            }    

            DBG_RB(local_stats.n_worker_executed++;)
                
            // create, save and close file
            std::ostringstream ss;
            ss << std::setw(10) << std::setfill('0') << n_measurement_saved;
            std::string str_n_measurement_saved = ss.str();
            std::string folder_path = SAVE_PATH;
            std::string file_name = "ch_measurement_";
            std::string full_file_path = folder_path + file_name + str_n_measurement_saved + ".bin";
            n_measurement_saved++;
            std::ofstream fout(full_file_path, std::ios::out | std::ios::binary);
            if(buffer2process == BUFFER0){
                for(size_t ch = 0; ch < n_channels; ch++)
                    fout.write((char*)&buffs0[ch][0], buffs0[ch].size()*sizeof(buffs0[ch][0]));
            }
            else{
                for(size_t ch = 0; ch < n_channels; ch++)
                    fout.write((char*)&buffs1[ch][0], buffs1[ch].size()*sizeof(buffs0[ch][0]));
            }
            fout.close();

            // we are done, make sure we enter wait loop
            buffer2process = NO_BUFFER;
    }
}
    
void show_debug_information_fifo(){
    local_stats.print_data("FIFO:");
}
    
static void print_data_init(){
    std::cout << "--------------------------" << std::endl;
    std::cout << "FIFO start statistics:" << std::endl;
    std::cout << "CH_MEASUREMENT_PER_SEC: " << CH_MEASUREMENT_PER_SEC << std::endl;
    std::cout << "CH_MEASUREMENT_LENGTH_IN_SAMPLES: " << CH_MEASUREMENT_LENGTH_IN_SAMPLES << std::endl;
    std::cout << "CH_MEASUREMENT_SAVE_PERIOD_SEC: " << CH_MEASUREMENT_SAVE_PERIOD_SEC << std::endl;
    std::cout << "CH_MEASUREMENT_SAVE_PERIOD: " << CH_MEASUREMENT_SAVE_PERIOD << std::endl;
    std::cout << "n_channels: " << n_channels << std::endl;
    std::cout << "n_bytes_per_item: " << n_bytes_per_item << std::endl;
    std::cout << "samp_rate: " << samp_rate << std::endl;
    std::cout << "n_samples_per_period: " << n_samples_per_period << std::endl;
    
    // how large will a single measurement be?
    unsigned long long measurement_size_bytes = n_channels*CH_MEASUREMENT_LENGTH_IN_SAMPLES*n_bytes_per_item;
    unsigned long long measurements_per_second_size_bytes = measurement_size_bytes*CH_MEASUREMENT_PER_SEC;
    unsigned long long measurements_per_file_size_bytes = measurement_size_bytes*CH_MEASUREMENT_SAVE_PERIOD;
    unsigned long long measurements_per_minute_size_bytes = measurements_per_second_size_bytes*60;
    
    std::cout << "measurement_size_bytes: " << measurement_size_bytes << std::endl;
    std::cout << "measurements_per_second_size_bytes: " << measurements_per_second_size_bytes << std::endl;
    std::cout << "measurements_per_file_size_bytes: " << measurements_per_file_size_bytes << std::endl;
    std::cout << "measurements_per_minute_size_bytes: " << measurements_per_minute_size_bytes << std::endl;
    std::cout << "--------------------------" << std::endl;    
}
}
