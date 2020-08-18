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

#ifndef CHANNELSOUNDER_RINGBUFFER_TX_H
#define CHANNELSOUNDER_RINGBUFFER_TX_H

#include <vector>
#include <atomic>

namespace channelsounder
{
/*!
 * Inits unit internally. Must be called first.
 *
 * num_channels_arg             in our case this is the number of rx antennas
 * num_bytes_per_item_arg       one item is one complex sample with real and imag, e.g. with data type "float" it is num_bytes_per_item_arg=8
 * max_items_per_packet_arg     depends on what uhd driver does, tries to fully utilize 10Gbit/s bandwidth of ethernet NIC, needed for size of internal static memory
 * return                       1 on success and 0 on failure
*/
int init_ringbuffer_tx(const size_t n_channels_arg, const size_t n_bytes_per_item_arg, const size_t max_items_per_packet_arg, const unsigned int samp_rate_arg);

/*!
 * Must be called initially with n_new_samples=0.
 * Breaks unit encapsulation, better solution needed.
 *
 * n_new_samples                number of new samples read per channel to pointers from last call
 * return                       vector of pointers pointing to internal static vectors, this is where uhd reads from
*/
std::vector<void*> get_ringbuffer_tx_pointers(const size_t n_new_samples);
    
/*!
 * Shows some stats of the ring buffer.
*/    
void show_debug_information_ringbuffer_tx();
}
 
#endif
