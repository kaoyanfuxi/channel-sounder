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

% define system parameters
samp_rate = 200e6;
fc = 800e6;

% create channel
ch = lib_rf_channel.rf_channel();
ch.index                    = 1;
ch.type                     = 'stochastical';
ch.amp                      = 1.0;
ch.noise                    = false;
ch.snr_db                   = 100;
ch.sto                      = 0;
ch.err_phase                = deg2rad(0);
ch.cfo                      = 0;
ch.s_samp_rate              = samp_rate;
ch.s_max_doppler        	= lib_rf_channel.convert_speed_kmh_2_maxdoppler_hz(500,fc);
ch.s_rayleigh_type          = 'Custom Exponential Decay';
ch.s_rayleigh_tau_mean      = 15*1/samp_rate;
ch.s_rayleigh_tau_rms       = 15*1/samp_rate;
ch.s_rayleigh_gains_yn      = true;
ch.s_random_source          = 'global';
ch.s_rayleigh_random_seed   = 11;
ch.s_noise_random_stream	= RandStream('mt19937ar','Seed', 22);
ch.init_stochastical_channel();

% call plot function of channel
ch.debug_simulate_plot();
ch.reset_random_streams('matlab_channel_object_only');

% create input
samples = zeros(1000,1);
samples(1) = 1;

% pass through rf channel
samples_ch = ch.pass_samples(samples, 123456);

% show some channel coeffs
len = 3;
display(samples_ch(1:len));

% check if matlab agrees (only direct path)
tmp = diag(ch.s_rayleigh_gains);
display(tmp(1:len));
