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

function [filenames, seq_file] = get_all_filenames(folderpath)

    filenames = dir(folderpath);
    
    % remove . and ..
    filenames(1) = [];
    filenames(1) = [];
    
    if numel(filenames) == 0
        error("No files found.");
    end    
    
    % separate seq file
    seq_file = filenames(end);
    filenames(end) = [];
end