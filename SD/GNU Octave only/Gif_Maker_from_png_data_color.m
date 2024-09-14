%images must be taken with color filter, in this order: RGBRGBRGB...
%the exposure must be set to auto all the time and the DashBoy allowed to
%adjust exposure between each filter.
clc
clear
pkg load image
%script to be run directly in the image folder
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
target_mp4_file='Output.mp4'; %target file for mp4, keep all image
target_gif_file='Output.gif'; %target file for animated gif
gif_deadtime=0.1;            %delay is seconds between pictures for animated gifs
scaling_factor=0.5;           %because images are 8x after powershell step
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%vidfile = VideoWriter(target_mp4_file,'MPEG-4');
%open(vidfile);
listing = dir('*.png');
for i=1:3:length(listing)
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
    R=R(:,:,1);
    G=G(:,:,1);
    B=B(:,:,1);
    frame(:,:,1)=R;
    frame(:,:,2)=G;
    frame(:,:,3)=B;
    %writeVideo(vidfile, frame);
    %[imind,map] = rgb2ind(cat(3,frame),256);
    [imind,map] = rgb2ind(cat(3,frame));
    if i==1
        imwrite(imind,map,target_gif_file,'gif', 'Loopcount',inf,'DelayTime',gif_deadtime);
    else
        imwrite(imind,map,target_gif_file,'gif','WriteMode','append','DelayTime',gif_deadtime);
    end
end
%close(vidfile)
disp('End of conversion, enjoy your fancy animations !')
