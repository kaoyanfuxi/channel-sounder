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

function [seq] = read_seq(seq_file, sys_param)

    seq = [];

    full_filepath = strcat(seq_file.folder,"/",seq_file.name);

    all_samples = lib_data_usrp.read_complex_binary(full_filepath, sys_param.data_type, 0, 0);

    % separate into channels
    all_samples_per_channel = reshape(all_samples, numel(all_samples)/sys_param.n_tx_channels, sys_param.n_tx_channels);
    
    % check if sequence makes sense
    for i=1:1:sys_param.n_tx_channels
        
        % extract one specific channel
        samples_channel = all_samples_per_channel(:,i);
        
        % how many copies are there?
        n_seq_cpy = numel(samples_channel)/sys_param.seq_len;
        
        % reshape so that we can compare sequences
        samples_channel = reshape(samples_channel,sys_param.seq_len,n_seq_cpy);
        
        % make sure each column is exactly the same
        for j=2:1:n_seq_cpy
            res = samples_channel(:,j-1) - samples_channel(:,j);
            if sum(res) ~= 0
                error("Two columns are not equal.");
            end
        end
        
        % generate matrix with single sequences
        if i==1
            seq = samples_channel(:,1);
        else
            seq = [seq, samples_channel(:,1)];
        end
    end
end

