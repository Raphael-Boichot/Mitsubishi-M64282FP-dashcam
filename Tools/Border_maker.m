a=imread('Border.png');

fid=fopen('prettyborder.h','w');
[hauteur, largeur, pro]=size(a);
a=a(:,:,1);
counter=0;
palette=unique(a)
a(a==palette(1))=0;
a(a==palette(2))=1;
a(a==palette(3))=2;
a(a==palette(4))=3;
a(17:128,17:144)=0;
fprintf(fid,'const unsigned char prettyborder[] = {');
for i=1:1:hauteur
    for j=1:1:largeur
        %if not(a(i,j)==4);
            counter=counter+1;
            fprintf(fid,'0x');
            fprintf(fid,'%X',a(i,j));
            fprintf(fid,',');
            if rem(counter,16)==0;fprintf(fid,'\n'); end;
        %end
    end
end
fseek(fid,-2,'cof');
fprintf(fid,'};');
fclose(fid);
imagesc(a)
