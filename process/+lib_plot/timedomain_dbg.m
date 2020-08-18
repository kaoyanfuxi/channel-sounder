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

function [] = timedomain_dbg(ch_meas_files, seq, sys_param)

    % TX
    figure(1)
    clf();
    n_ch_max = max(sys_param.n_tx_channels, sys_param.n_rx_channels);
    for ch_tx=1:1:sys_param.n_tx_channels
        
        n_seq = numel(seq(:, ch_tx));
        
        subplot(n_ch_max,1,ch_tx);
        plot(real(seq(:, ch_tx)));
        hold on
        plot(imag(seq(:, ch_tx)),'r');
        
        title("TX Samples of Channel Index " + num2str(ch_tx-1));
        legend('real','imag');
        xlabel("Sample Index");
        ylabel("Amplitude");
        axis([-n_seq*0.1 n_seq*1.1 -1 1]);
        grid on
    end
    
    idx = 6;
    cuout_len = 5*sys_param.ch_measurement_len;

    % extract complex samples of any measurements
    complex_samples = ch_meas_files(6).complex_samples;    
    
    % RX
    figure(2)
    clf();    
    for ch_rx=1:1:sys_param.n_rx_channels
        
        subplot(n_ch_max,1,ch_rx);
        plot(real(complex_samples(1:cuout_len, ch_rx)));
        hold on
        plot(imag(complex_samples(1:cuout_len, ch_rx)),'r');
        
        title("RX Samples of Channel Index " + num2str(ch_rx-1));
        legend('real','imag');
        xlabel("Sample Index");
        ylabel("Amplitude");
        axis([-cuout_len*0.1 cuout_len*1.1 -0.7 0.7]);
        grid on
    end
end

