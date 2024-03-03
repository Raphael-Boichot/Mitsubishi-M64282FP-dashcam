clc
clear

disp('------------------------------------------')
disp('|Beware, this code is for Matlab ONLY !!! |')
disp('------------------------------------------')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
target_directory='';   %source directory to process
target_mp4_file='Test.mp4'; %target file for mp4, keep all image
target_gif_file='Test.gif'; %target file for animated gif
gif_deadtime=1;               %delay is seconds between pictures for animated gifs
gif_skip=1;                   %keep every 1 out of gif_skip image for gif
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

vidfile = VideoWriter(target_mp4_file,'MPEG-4');
open(vidfile);
listing = dir([target_directory,'*.png']);
for i=1:1:length(listing)
    name=[target_directory,listing(i).name];
    disp(['Processing ',target_directory,listing(i).name]);
    frame=imread(name);
    [height, width, null]=size(frame);
    frame=imresize(frame,[576 640],'nearest');
    writeVideo(vidfile, frame);
    [imind,map] = rgb2ind(cat(3,frame),256);
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