clc
clear
a=imread('test.png');

fid=fopen('prettyborder.h','w');
[hauteur, largeur, pro]=size(a);
a=double(a(:,:,1));
palette=unique(a)
old_pixel_value=220;
new_pixel_value=255;
b=not(a==old_pixel_value).*a+(a==old_pixel_value)*new_pixel_value;
imagesc(a)
imwrite(uint8(b), 'test.png');
palette=unique(b)