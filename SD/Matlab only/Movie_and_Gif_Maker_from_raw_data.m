clc
clear
%script to be run in the ./TL or ./MS folder
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%User parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
upscaling_factor=4;%...upscaling factor
delay=0.05;%delay between pictures for animated gifs
gif_skip=1;%includes only one image over gif_skip into the gif
%filenames are automatically generated
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

listing = dir('*.raw');
for i=1:1:length(listing)
    name=listing(i).name
    vidfile = VideoWriter([name(1:end-4),'.mp4'],'MPEG-4');
    open(vidfile);
    fileID = fopen(name);
    data_raw=fread(fileID);
    data=char(data_raw);
    height=data(8);
    width=data(7);
    data=convertCharsToStrings(data);
    k = strfind(data,'RAWDAT');
    mkdir(['./',name(1:end-4)]);
    for i=1:1:length(k)-1
        offset=k(i)+16;
        pixel_data=data_raw(offset:offset+(height)*width-1);
        %         minimum=min(min(pixel_data(128:end-128*8)));
        %         maximum=max(max(pixel_data(128:end-128*8)));
        %         pixel_data=(pixel_data-minimum)*(255/(maximum-minimum));
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
            if rem(i,gif_skip)==0
                imwrite(imind,map,[name(1:end-4),'.gif'],'gif','WriteMode','append','DelayTime',delay);
            end
        end
        imwrite(scaled,['./',name(1:end-4),'/',name(1:end-4),num2str(i,'%04d'),'.png'])
    end
    close(vidfile)
end

