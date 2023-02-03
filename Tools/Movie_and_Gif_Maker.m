clc
clear
upscaling_factor=4;%...upscaling factor
delay=0.05;%delay between pictures for animated gifs
vidfile = VideoWriter('Output.mp4','MPEG-4');
open(vidfile);
listing = dir('*.bmp');
for i=1:1:length(listing)
    name=listing(i).name
    frame=imread(name);
    data(:,:,i)=frame;
    scaled=imresize(frame,upscaling_factor,'nearest');
    writeVideo(vidfile, scaled);
    [imind,map] = rgb2ind(cat(3,scaled,scaled,scaled),256);
    if i==1
        imwrite(imind,map,'Output.gif','gif', 'Loopcount',inf,'DelayTime',delay);
    else
        imwrite(imind,map,'Output.gif','gif','WriteMode','append','DelayTime',delay);
    end
end
close(vidfile)
average=mean(data,3);
colormap gray
minimum=min(min(min(average)));
maximum=max(max(max(average)));
average=(average-minimum)*(255/(maximum-minimum));
average=uint8(average);
average=imresize(average,upscaling_factor,'nearest');
imwrite(average,'Output.png')