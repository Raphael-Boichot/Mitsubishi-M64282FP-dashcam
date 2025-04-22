clc
clear
%script to be run directly in the image folder
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
target_mp4_file='Output.mp4'; %target file for mp4, keep all image
target_gif_file='Output.gif'; %target file for animated gif
gif_deadtime=0.05;            %delay is seconds between pictures for animated gifs
gif_skip=2;                   %keep every 1 out of gif_skip image for gif
scaling_factor=0.5;           %because images are 8x after powershell step
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

vidfile = VideoWriter(target_mp4_file,'MPEG-4');
open(vidfile);
listing = dir('*.png');
for i=1:1:length(listing)
    name=listing(i).name;
    disp(['Processing ',listing(i).name]);
    frame=imread(name);
    [height, width, null]=size(frame);
    frame=imresize(frame,scaling_factor,'nearest');
    writeVideo(vidfile, frame);
    [imind,map] = rgb2ind(frame,256);
    if i==1
        imwrite(imind,map,target_gif_file,'gif', 'Loopcount',inf,'DelayTime',gif_deadtime);
    else
        if rem(i,gif_skip)==0
        imwrite(imind,map,target_gif_file,'gif','WriteMode','append','DelayTime',gif_deadtime);
        end
    end
end
close(vidfile)
disp('End of conversion, enjoy your fancy animations !')