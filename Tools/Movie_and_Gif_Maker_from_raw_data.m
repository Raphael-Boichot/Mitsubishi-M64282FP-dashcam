clc
clear
upscaling_factor=4;%...upscaling factor
delay=0.05;%delay between pictures for animated gifs

listing = dir('*.raw');
for i=1:1:length(listing)
    name=listing(i).name
    vidfile = VideoWriter([name(1:end-4),'.mp4'],'MPEG-4');
    open(vidfile);
    fileID = fopen(name);
    data_raw=fread(fileID);
    data=char(data_raw);
    height=str2double(data(14:16));
    width=str2double(data(10:12));
    data=convertCharsToStrings(data);
    k = strfind(data,'RAW_8BIT_');
    for i=1:1:length(k)-1
        offset=k(i)+32;
        pixel_data=data_raw(offset:offset+height*width-1);
        pixels=uint8(rot90(reshape(pixel_data,width,height),3));
        pixels=fliplr(pixels);
        imshow(pixels);
        drawnow
        scaled=imresize(pixels,upscaling_factor,'nearest');
        writeVideo(vidfile, scaled);
        [imind,map] = rgb2ind(cat(3,scaled,scaled,scaled),256);
        if i==1
            imwrite(imind,map,[name(1:end-4),'.gif'],'gif', 'Loopcount',inf,'DelayTime',delay);
        else
            imwrite(imind,map,[name(1:end-4),'.gif'],'gif','WriteMode','append','DelayTime',delay);
        end
        
        
    end
    
   
    close(vidfile)
end

