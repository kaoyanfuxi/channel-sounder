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

function [] = check_seq(sys_param)

    if sys_param.samp_rate/sys_param.ch_measurement_per_sec <= sys_param.ch_measurement_len
        error("Length of single measurement must be shorer than spacing between measurements.");
    end

    % we assume WLAN 802.11
    % cyclic prefix is 25% of one symbol
    % the physical dimension of a WLAN system is about 100m
    t_cyclic_prefix_80211 = 64*0.25/20e6;
    max_spatial_area_80211 = 100;
    
    % how long will the rf channel be in our case?
    t_rf_channel = t_cyclic_prefix_80211*sys_param.max_spatial_area/max_spatial_area_80211;
    
    % how many samples do we need at least?
    T = 1/sys_param.samp_rate;
    n_min_samples = ceil(t_rf_channel/T);
    
    utilization = sys_param.ch_measurement_per_sec*sys_param.ch_measurement_len/sys_param.samp_rate;
    
    fprintf("Sampling Rate: %f MS/s\n", sys_param.samp_rate/1e6);
    fprintf("T: %f us\n", T/1e-6);
    fprintf("RF channel spatial area: %f m\n", sys_param.max_spatial_area);
    fprintf("RF channel time length: %f us\n", t_rf_channel/1e-6);
    fprintf("Number of samples required: %f\n", n_min_samples);
    fprintf("Number of samples used: %f\n", sys_param.ch_measurement_len);
    fprintf("Measurements per second: %f\n", sys_param.ch_measurement_per_sec);
    fprintf("Utilization: %f percentage\n", utilization*100);    
    
    if sys_param.ch_measurement_len < n_min_samples
        error("Length of sequence is too short.");
    end
end

