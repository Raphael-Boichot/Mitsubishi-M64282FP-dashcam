clc
clear
% Script to be run directly in the image folder
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
target_mp4_file = 'Output.mp4';
target_gif_file = 'Output.gif';
gif_deadtime = 0.1;           
gif_skip = 1;                  
scaling_factor = 0.5;
v_border = 128; 
h_border = 128;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
listing = dir('*.png');
if length(listing) < 3, error('Not enough images.'); end
size_list = length(listing) - rem(length(listing), 3);

% Create folder for full-size frames
if ~exist('RGB_frames', 'dir')
    mkdir('RGB_frames');
end

% --- PASS 1: Calculate Weights ---
disp('Analyzing frames...');
sum_R = 0; sum_G = 0; sum_B = 0; count = 0;
for i = 1:3:size_list
    temp_R = imread(listing(i).name);   temp_R = temp_R(:,:,1);
    temp_G = imread(listing(i+1).name); temp_G = temp_G(:,:,1);
    temp_B = imread(listing(i+2).name); temp_B = temp_B(:,:,1);
    
    sum_R = sum_R + mean(double(temp_R), 'all');
    sum_G = sum_G + mean(double(temp_G), 'all');
    sum_B = sum_B + mean(double(temp_B), 'all');
    count = count + 1;
end
avg_R = sum_R / count; avg_G = sum_G / count; avg_B = sum_B / count;
max_avg = max([avg_R, avg_G, avg_B]);
auto_weights = [max_avg/avg_R, max_avg/avg_G, max_avg/avg_B];

% --- PASS 2: Processing ---
vidfile = VideoWriter(target_mp4_file, 'MPEG-4');
vidfile.FrameRate = 10;
open(vidfile);
gif_counter = 0;

for i = 1:3:size_list
    gif_counter = gif_counter + 1;
    
    % Force 2D read
    R_raw = double(imread(listing(i).name));   R_raw = R_raw(:,:,1);
    G_raw = double(imread(listing(i+1).name)); G_raw = G_raw(:,:,1);
    B_raw = double(imread(listing(i+2).name)); B_raw = B_raw(:,:,1);
    
    % Apply white balance
    R = min(255, R_raw * auto_weights(1));
    G = min(255, G_raw * auto_weights(2));
    B = min(255, B_raw * auto_weights(3));
    
    % Restore borders
    R(1:v_border, :) = R_raw(1:v_border, :); R(end-v_border+1:end, :) = R_raw(end-v_border+1:end, :);
    R(:, 1:h_border) = R_raw(:, 1:h_border); R(:, end-h_border+1:end) = R_raw(:, end-h_border+1:end);
    G(1:v_border, :) = G_raw(1:v_border, :); G(end-v_border+1:end, :) = G_raw(end-v_border+1:end, :);
    G(:, 1:h_border) = G_raw(:, 1:h_border); G(:, end-h_border+1:end) = G_raw(:, end-h_border+1:end);
    B(1:v_border, :) = B_raw(1:v_border, :); B(end-v_border+1:end, :) = B_raw(end-v_border+1:end, :);
    B(:, 1:h_border) = B_raw(:, 1:h_border); B(:, end-h_border+1:end) = B_raw(:, end-h_border+1:end);
    
    % Create full-size frame
    full_frame = uint8(cat(3, R, G, B));
    
    % Save to RGB_frames folder
    imwrite(full_frame, fullfile('RGB_frames', sprintf('frame_%04d.png', gif_counter)));
    
    % Resize for video/GIF
    target_size = [floor(size(R_raw, 1) * scaling_factor), floor(size(R_raw, 2) * scaling_factor)];
    R_s = uint8(imresize(R, target_size, 'nearest'));
    G_s = uint8(imresize(G, target_size, 'nearest'));
    B_s = uint8(imresize(B, target_size, 'nearest'));
    frame_scaled = cat(3, R_s, G_s, B_s);
    
    % Write Video
    writeVideo(vidfile, frame_scaled);
    
    % GIF generation
    [imind, map] = rgb2ind(frame_scaled, 256);
    if i == 1
        imwrite(imind, map, target_gif_file, 'gif', 'Loopcount', inf, 'DelayTime', gif_deadtime);
    elseif rem(gif_counter, gif_skip) == 0
        imwrite(imind, map, target_gif_file, 'gif', 'WriteMode', 'append', 'DelayTime', gif_deadtime);
    end
end
close(vidfile);
disp('Conversion complete.');