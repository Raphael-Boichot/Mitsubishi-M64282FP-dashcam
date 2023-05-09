a=imread('Border.png');

fid=fopen('prettyborder.h','w');
[hauteur, largeur, pro]=size(a);
a=a(:,:,1);
counter=-1;
disp_counter=0;
palette=unique(a)
a(a==palette(1))=0;
a(a==palette(2))=1;
a(a==palette(3))=2;
a(a==palette(4))=3;
a(17:128,17:144)=0;
old_value=a(1,1);
fprintf(fid,'const unsigned char prettyborder[] = {');
for i=1:1:hauteur
    for j=1:1:largeur
        %The code uses a kind of cheap RLE compression
        counter=counter+1;
        value=a(i,j);
        if ((not(old_value==value))||(counter==255));
            disp_counter=disp_counter+1;
            
            fprintf(fid,'0x');
            if (counter<0xF);fprintf(fid,'0');end;
            fprintf(fid,'%X',counter);
            fprintf(fid,',');
            
            counter=0;
            
            fprintf(fid,'0x');
            if (old_value<0xF);fprintf(fid,'0');end;
            fprintf(fid,'%X',old_value);
            fprintf(fid,',');
            
            if rem(disp_counter,16)==0;fprintf(fid,'\n'); end;
        end;
        old_value=value;
    end
end

counter=counter+1;
disp_counter=disp_counter+1;
fprintf(fid,'0x');
if (counter<0xF);fprintf(fid,'0');end;
fprintf(fid,'%X',counter);
fprintf(fid,',');
counter=0;
fprintf(fid,'0x');
if (a(i,j)<0xF);fprintf(fid,'0');end;
fprintf(fid,'%X',a(i,j));
fprintf(fid,',');
if rem(disp_counter,16)==0;fprintf(fid,'\n'); end;


fseek(fid,-1,'cof');
fprintf(fid,'};');
fclose(fid);
imagesc(a)
