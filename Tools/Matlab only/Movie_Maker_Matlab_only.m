clc
clear

disp('------------------------------------------')
disp('|Beware, this code is for Matlab ONLY !!! |')
disp('------------------------------------------')

vidfile = VideoWriter('Output.mp4','MPEG-4');
open(vidfile);
listing = dir('*.png');
for i=1:1:length(listing)
    name=listing(i).name
    frame=imread(name);
    %frame=rot90(frame);%90° rotation
    %frame=rot90(frame);%90° rotation
    data(:,:,i)=frame(:,:,1);
    writeVideo(vidfile, frame);
end
close(vidfile)
average=mean(data,3);
minimum=min(min(min(average)));
maximum=max(max(max(average)));
average=(average-minimum)*(255/(maximum-minimum));
average=uint8(average);
imwrite(average,'Output.png')
imshow(average);