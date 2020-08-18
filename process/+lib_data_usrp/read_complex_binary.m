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

% source: https://github.com/UpYou/gnuradio-tools/blob/master/matlab/read_complex_binary.m
%
% Changes: Samples can be skipped, added data types.

function v = read_complex_binary(filename, data_type, count, skip_samples)

    % what is the size of the input data in bytes
    switch lower(data_type)
        case {'double', 'int64', 'uint64'}
            byte_of_data_type = 8;
        case {'float', 'single', 'int32', 'uint32'}
            byte_of_data_type = 4;
        case {'char', 'int16', 'uint16'}
            byte_of_data_type = 2;
        case {'logical', 'int8', 'uint8'}
            byte_of_data_type = 1;
    otherwise
        warning('JSimon:sizeof:BadClass', 'Class "%s" is not supported.', data_type);
        byte_of_data_type = NaN;
    end

    f = fopen(filename, 'rb');
    
    if (f < 0)
        
        error('ERROR: Cannot read file with path: %s', filename);
        
    else
        
        % check if skip samples and read samples are smaller than actual file size
        filebytes = dir(filename);
        
        % 0 indicates that we want the maximum file size
        if count == 0
            if mod(filebytes.bytes,byte_of_data_type) ~= 0
                error('ERROR: filesize in bytes not a multiple of item size');
            end
            count = filebytes.bytes/byte_of_data_type;
        end

        % 2 -> real and imag
        if((skip_samples + count)*byte_of_data_type > filebytes.bytes)
            fprintf('\nFilesize in bytes:            %d\n', filebytes.bytes);
            fprintf('\nSkip and count size in bytes: %d\n', (skip_samples + count)*2*byte_of_data_type);       
            error('ERROR: Skip and count are larger than file.');
        end 

        % skip complex samples
        if(skip_samples > 0)
            fseek(f, skip_samples*byte_of_data_type,'bof');
        end

        % read in bytes
        t = fread(f, [2, count], data_type);
        fclose (f);
        v = t(1,:) + t(2,:)*1i;
        [r,c] = size(v);
        v = reshape (v, c, r);      
    end
end
