%images must be taken with color filter, in this order: RGBRGBRGB...
%the exposure must be set to auto all the time and the DashBoy allowed to
%adjust exposure between each filter.
clc
clear
%script to be run directly in the image folder
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
target_mp4_file='Output.mp4'; %target file for mp4, keep all image
target_gif_file='Output.gif'; %target file for animated gif
gif_deadtime=0.04;            %delay is seconds between pictures for animated gifs, 25 fps
gif_skip=2;                   %keep every 1 out of gif_skip image for gif
scaling_factor=0.25;          %because images are 8x after powershell step
color_weight=[1 1 1]        %[R G B] weights to get a gray image when taking a white screen in photo in my case
%to get these values, take pictures of a white scene with the three filters, note the exposure times and divide by the minimal value
%these corresponds to my filters but yours can vary
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

vidfile = VideoWriter(target_mp4_file,'MPEG-4');
open(vidfile);
listing = dir('*.png');
gif_counter=0;
size_list=length(listing)-rem(length(listing),3);

for i=1:3:size_list
    gif_counter=gif_counter+1;
    name=listing(i).name;
    disp(['Processing ',listing(i).name]);
    R=imread(name);
    name=listing(i+1).name;
    disp(['Processing ',listing(i+1).name]);
    G=imread(name);
    name=listing(i+2).name;
    disp(['Processing ',listing(i+2).name]);
    B=imread(name);
    [height, width, null]=size(R);
    R=imresize(R,scaling_factor,'nearest');
    G=imresize(G,scaling_factor,'nearest');
    B=imresize(B,scaling_factor,'nearest');
    R=uint8(R(:,:,1)*color_weight(1));
    G=uint8(G(:,:,1)*color_weight(2));
    B=uint8(B(:,:,1)*color_weight(3));
    frame(:,:,1)=R;
    frame(:,:,2)=G;
    frame(:,:,3)=B;
    writeVideo(vidfile, frame);
    [imind,map] = rgb2ind(frame,256);
    if i==1
        imwrite(imind,map,target_gif_file,'gif', 'Loopcount',inf,'DelayTime',gif_deadtime);
    else
        if rem(gif_counter,gif_skip)==0
            imwrite(imind,map,target_gif_file,'gif','WriteMode','append','DelayTime',gif_deadtime);
        end
    end
end
close(vidfile)
disp('End of conversion, enjoy your fancy animations !')