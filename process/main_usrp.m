%
% This program is free software: you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation, either version 3 of the License, or
% (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License
% along with this program.  If not, see <http://www.gnu.org/licenses/>.
%

clear all;
close all;

% variables
sys_param.folderpath = "../data";
sys_param.data_type = "float";
sys_param.samp_rate = 125e6;
sys_param.n_rx_channels = 4;
sys_param.n_tx_channels = 4;
sys_param.ch_measurement_per_sec = 1000;
sys_param.ch_measurement_len = 520;
sys_param.ch_measurement_save_period_sec = 5;
sys_param.seq_len = 125;

% first find all recorded files
[filenames, seq_file] = lib_data_usrp.get_all_filenames(sys_param.folderpath);
n_files = numel(filenames);
if n_files == 0
    error("No files found!");
else
    fprintf("Read %d measurement files.\n", n_files);
end

% read seq file
seq = lib_data_usrp.read_seq(seq_file, sys_param);

% create vector of elements, one element per file
ch_meas_files(n_files,1) = lib_data_usrp.measurement_file();
for n=1:n_files
    ch_meas_files(n) = lib_data_usrp.measurement_file(filenames(n), sys_param);
end

% show some samples in time domain
lib_plot.timedomain_dbg(ch_meas_files, seq, sys_param);

% get time tag for each channel measurement
% TODO

% extract channel impulse responses by correlating channel measurements with sequence
% TODO

% transform channel measurements into frequency domain
% TODO

% sanity-check if channel measurements are correlated (change slowly over time)
% TODO

% save data in correct data format
% TODO

