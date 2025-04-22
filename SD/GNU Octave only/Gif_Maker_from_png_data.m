clc
clear
%script to be run directly in the image folder

disp('-----------------------------------------------------------')
disp('|Beware, this code is for GNU Octave ONLY !!!             |')
disp('-----------------------------------------------------------')
pkg load image

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
target_gif_file='Output.gif'; %target file for animated gif
gif_deadtime=0.05;            %delay is seconds between pictures for animated gifs
gif_skip=1;                   %keep every 1 out of gif_skip image for gif
scaling_factor=1;           %because images are 8x after powershell step
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

listing = dir('*.png');
for i=1:1:length(listing)
    name=listing(i).name;
    disp(['Processing ',listing(i).name]);
    frame=imread(name);
    [height, width, null]=size(frame);
    frame=imresize(frame,scaling_factor,'nearest');
    [imind,map] = rgb2ind(frame);
    if i==1
        imwrite(imind,map,target_gif_file,'gif', 'Loopcount',inf,'DelayTime',gif_deadtime,'Compression','bzip');
    else
        if rem(i,gif_skip)==0
        imwrite(imind,map,target_gif_file,'gif','WriteMode','append','DelayTime',gif_deadtime,'Compression','bzip');
        end
    end
end
disp('End of conversion, enjoy your fancy animations !')
