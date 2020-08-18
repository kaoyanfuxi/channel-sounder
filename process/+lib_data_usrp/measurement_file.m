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

classdef measurement_file
    %MEASUREMENT_FILE Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        name
        folder
        date
        bytes
        full_filepath
        
        sys_param_cpy
        
        complex_samples
    end
    
    methods
        function obj = measurement_file(file_struct, sys_param)
            if nargin == 0
                return;
            end
            obj.name    = file_struct.name;
            obj.folder  = file_struct.folder;
            obj.date    = file_struct.date;
            obj.bytes   = file_struct.bytes;
            
            obj.full_filepath = strcat(obj.folder,"/",obj.name);
            
            obj.sys_param_cpy = sys_param;
            
            obj.complex_samples = obj.read_samples();
        end
        
        function complex_samples = read_samples(obj)
            complex_samples = lib_data_usrp.read_complex_binary(obj.full_filepath, obj.sys_param_cpy.data_type, 0, 0);
            
            % channels are concatenated
            n_complex_samples = numel(complex_samples);
            n_complex_samples_per_channel = n_complex_samples/obj.sys_param_cpy.n_rx_channels;
            
            % separate into channels
            complex_samples = reshape(complex_samples, n_complex_samples_per_channel, obj.sys_param_cpy.n_rx_channels);
            
            % sanity check
            len = obj.sys_param_cpy.ch_measurement_per_sec;
            len = len * obj.sys_param_cpy.ch_measurement_len;
            len = len * obj.sys_param_cpy.ch_measurement_save_period_sec;
            if len ~= numel(complex_samples(:,1))
                error('Incorrect number of samples per channel per saved file.');
            end 
        end
    end
end

