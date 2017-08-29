function [volt, tau, cfg] = lcm_load(fname)
% Load LCM generated voltage data 
%   voltage: the neuronal voltage data, with the size of 'volt(tau, elmt, neur)'
%   tau: a vector containing the time points of the voltage data
%   cfg: the parameter configuration read from the file

tau = NaN;
volt = NaN;
cfg = NaN;

if nargin == 0
    display('ERROR! A file name must be provided!');
    display('Usage: [volt, tau, cfg] = lcm_load(fname).');
    return;
end

if exist(fname, 'file') == 0
    disp(['ERROR! Cannot find the file "', fname, '"']);
    return;
end

%first of all open the file in binary mode
fp = fopen(fname, 'r');

%read the header
lcm = fread(fp, 1024, '*char');

neur_num=NaN;
elmt_num=NaN;
cfg_pos=NaN;
cfg_len=NaN;
num_size=NaN;
block_size=NaN;
block_num=NaN;
data_pos=NaN;
data_len=NaN;

%parsing the header
% the header contains parameter values in the formate of
% paraName = paraVal ; //comments
paraLst = regexp(lcm', '(?<paraName>\w*)\s*=\s*(?<paraVal>\w*)\s*;', 'tokens');

%read the parameter values
for idx=1:length(paraLst)
    if strcmpi(paraLst{idx}{1}, 'NEUR_NUM')
        neur_num = str2double(paraLst{idx}{2});
    elseif strcmpi(paraLst{idx}{1}, 'ElMT_NUM')
        elmt_num =  str2double(paraLst{idx}{2}); 
    elseif strcmpi(paraLst{idx}{1}, 'CFG_POS')
        cfg_pos =  str2double(paraLst{idx}{2});
    elseif strcmpi(paraLst{idx}{1}, 'CFG_LEN')
        cfg_len =  str2double(paraLst{idx}{2}); 
    elseif strcmpi(paraLst{idx}{1}, 'NUM_SIZE')
        num_size =  str2double(paraLst{idx}{2}); 
    elseif strcmpi(paraLst{idx}{1}, 'BLOCK_SIZE')
        block_size =  str2double(paraLst{idx}{2}); 
    elseif strcmpi(paraLst{idx}{1}, 'BLOCK_NUM')
        block_num =  str2double(paraLst{idx}{2}); 
    elseif strcmpi(paraLst{idx}{1}, 'DATA_POS')
        data_pos =  str2double(paraLst{idx}{2}); 
    elseif strcmpi(paraLst{idx}{1}, 'DATA_LEN')
        data_len =  str2double(paraLst{idx}{2}); 
    end
end

if(block_size*block_num ~= data_len)
    disp(['ERROR! Information in the head is not consitent! block_size = ' ...
        num2str(block_size), ', block_num = ', num2str(block_num), ...
        ', data_len = ', num2str(data_len)]);
    return;
end

%the data were written in float (C++), which is equivalent to single or
%float32 in matlab
if(num_size ~= 4)
    disp(['ERROR! The size of numbers is not euqavelent to 4 (num_size = ', ...
        num2str(num_size),')!']);
    return;
end

disp([num2str(block_num), ' blocks of data are found in the file!']);

%move the pointer to the beginning of the configuration section
fseek(fp, cfg_pos, 'bof');

%read the configuration section
cfg = fread (fp, cfg_len, '*char');

tau = zeros(block_num, 1);
volt = zeros(block_num, elmt_num, neur_num);
for idx=1:block_num
    %move the pointer to the beginning of the data block
    fseek(fp, data_pos + (idx-1)*block_size, 'bof');
    %read the first number, tau
    tau(idx) = fread(fp, 1, 'single');
    %read the voltage data
    buff = fread(fp, neur_num*elmt_num, 'single');
    volt(idx, :, :) = reshape(buff, neur_num, elmt_num)';
    %voltage data order: elmt1_neur1, elmt1_neur2, elmt1_neur3, ...
    %                    elmt2_neur1, elmt2_neur2, elmt2_neur3, ...
    if(volt(idx, 1, 2) ~= buff(2) || volt(idx, 2, 1) ~= buff(neur_num+1) )
        disp('ERROR! the index of the voltage data is not right!');
        return;
    end
    %there is a byte of 0 (=='\0' of C ++) at the end of each data block
    ch = fread(fp, 1, 'uint8');
    if(ch ~= 0 )
        disp(['ERROR! Data block ', num2str(idx), ' is not properly ended! (ch = ', ...
            num2str(ch),')']);
        break;
    end
end
fclose(fp);
return;
