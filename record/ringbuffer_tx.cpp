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
#include <stdlib.h>
#include <math.h>

#include "debug.h"
#include "config.h"
#include "ringbuffer_tx.h"

//#define ONE_AND_MINUS_ONE
#define SINE_1MHZ

#define SCALE_SC16  4096.0f
#define SCALE_FC32  0.5f

namespace channelsounder
{
static size_t n_channels;               // number of channels/antennas
static size_t n_bytes_per_item;         // size of complex sample
static size_t max_items_per_packet;     // maximum number of samples requested by uhd driver
static unsigned int samp_rate;          // S/s

static size_t n_seq;                    // number of sequence repetitions
static size_t n_seq_len;                // length of the sequence in complex samples

static size_t n_samples;                // at which samples index within the sequence are we?

// the static memory uhd will read from
// columns: number of rx channels (antennas)
// rows: container for samples
static std::vector<std::vector<char>> buffs0;

// actual output given to uhd, points to buffs0 and buffs1
static std::vector<void*> buffs;
    
static struct stats local_stats;

// after initializing buffer we generate the actual sequence
static void generate_sequence();
static void print_data_init();

int init_ringbuffer_tx(const size_t n_channels_arg, const size_t n_bytes_per_item_arg, const size_t max_items_per_packet_arg, const unsigned int samp_rate_arg){
    n_channels = n_channels_arg;
    n_bytes_per_item = n_bytes_per_item_arg;
    max_items_per_packet = max_items_per_packet_arg;
    samp_rate = samp_rate_arg;

#ifdef ONE_AND_MINUS_ONE
    n_seq_len = 4000;
#endif    
#ifdef SINE_1MHZ
    n_seq_len = samp_rate/1000000;
#endif
    
    n_samples = 0;
    
    // how often do we need to repeat the sequence?
    n_seq = max_items_per_packet/n_seq_len + 2;
    
    // initialize buffers and pointers
    std::vector<char> buff_template(n_seq * n_seq_len * n_bytes_per_item);
    for (size_t ch = 0; ch < n_channels; ch++){
        // create one row for each channel/antenna
        buffs0.push_back(buff_template);
        buffs.push_back(&buffs0[ch].front());
    }
    
    generate_sequence();
    print_data_init();
    local_stats.reset();
    
    return 1;
}
    
std::vector<void*> get_ringbuffer_tx_pointers(const size_t n_new_samples){
    DBG_RB(local_stats.n_samples_total += n_new_samples;)
    n_samples += n_new_samples;
    n_samples = n_samples % n_seq_len;

    unsigned int byte_offset = n_samples*n_bytes_per_item;
    for (size_t ch = 0; ch < n_channels; ch++)
        buffs[ch] = static_cast<void*>(&buffs0[ch][byte_offset]);
    
    return buffs;
}
    
void show_debug_information_ringbuffer_tx(){
    local_stats.print_data("Ringbuffer TX:");
}

static void generate_sequence(){
    
    // single sequence without repetitions
    std::vector<std::vector<float>> seq(n_channels);
    
    for(int ch=0; ch<n_channels; ch++){
        
        for(int j=0; j<n_seq_len; j++){
            
#ifdef ONE_AND_MINUS_ONE
            float real = 2*(rand() % 2) - 1;
            float imag = 2*(rand() % 2) - 1;
#endif            
#ifdef SINE_1MHZ
            double samp_rate_d = (double) samp_rate;
            double T = 1.0/samp_rate_d;
            double t = T* (double) j;
            double f = 1e6 + 1e6 * (double) ch;   // we variate the frequency for each channel
            double pi = 3.141592654;
            float real = cos(2.0*pi*f*t);
            float imag = sin(2.0*pi*f*t);
#endif
            seq[ch].push_back(real);
            seq[ch].push_back(imag);
        }
    }
    
    // repeat the sequence
    for(int ch=0; ch<n_channels; ch++){
        std::vector<float> cpy = seq[ch];
        for(int j=0; j<n_seq; j++)
            std::copy(cpy.begin(), cpy.end(), std::back_inserter(seq[ch]));
    }
    
    // sanity check the size
    for(int ch=0; ch<n_channels; ch++){
        unsigned int size_in_bytes_0 = buffs0[ch].size()*sizeof(buffs0[ch][0]);
        unsigned int size_in_bytes_1 = seq[ch].size()*sizeof(seq[ch][0]);
        if(size_in_bytes_0 != size_in_bytes_0)
            std::cerr << "ERROR: Sizes are not correct." << std::endl;
    }
    
    // finally copy all samples into desired buffer
    // sc16
    if (n_bytes_per_item == 4){
        
        std::vector<int16_t*> buff_s16;

        for(int i=0; i<n_channels; i++){
            
            buff_s16.push_back(static_cast<int16_t*>(buffs[i]));
            
            for(int j=0; j<n_seq*n_seq_len; j++){                
                buff_s16[i][2*j] = (int16_t) (SCALE_SC16*seq[i][2*j]);
                buff_s16[i][2*j+1] = (int16_t) (SCALE_SC16*seq[i][2*j+1]);
            }
        }
    }
    // fc32
    else if(n_bytes_per_item == 8){
        
        std::vector<float*> buff_f32;
        
        for(int i=0; i<n_channels; i++){
            
            buff_f32.push_back(static_cast<float*>(buffs[i]));
            
            for(int j=0; j<n_seq*n_seq_len; j++){
                buff_f32[i][2*j] = SCALE_FC32*seq[i][2*j];
                buff_f32[i][2*j+1] = SCALE_FC32*seq[i][2*j+1];
            }
        }
    }
    else{
        std::cerr << "ringbuffer_tx: Unknown data type." << std::endl;
    }

    // save the sequence
    std::string folder_path = SAVE_PATH;
    std::string file_name = "seq";
    std::string full_file_path = folder_path + file_name + ".bin";
    std::ofstream fout(full_file_path, std::ios::out | std::ios::binary);
    for(size_t ch = 0; ch < n_channels; ch++)
        fout.write((char*)&buffs0[ch][0], buffs0[ch].size()*sizeof(buffs0[ch][0]));
    fout.close();    
}
    
static void print_data_init(){
    std::cout << "--------------------------" << std::endl;
    std::cout << "Ringbuffer TX start:" << std::endl;
    std::cout << "n_channels: " << n_channels << std::endl;
    std::cout << "n_bytes_per_item: " << n_bytes_per_item << std::endl;
    std::cout << "max_items_per_packet: " << max_items_per_packet << std::endl;
    std::cout << "samp_rate: " << samp_rate << std::endl;
    std::cout << "n_seq_len: " << n_seq_len << std::endl;    
    std::cout << "n_seq: " << n_seq << std::endl;
    std::cout << "buffs0.size(): " << buffs0.size() << std::endl;
    for (size_t ch = 0; ch < n_channels; ch++){
        std::cout << "buffs0[" << ch << "].size(): " << buffs0[ch].size() << std::endl;
    }
    std::cout << "n_seq*n_seq_len*n_bytes_per_item: " << n_seq*n_seq_len*n_bytes_per_item << std::endl;
    std::cout << "--------------------------" << std::endl;        
}
}