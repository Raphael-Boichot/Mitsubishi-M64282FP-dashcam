%Made with CHATGPT
a = imread('palette.png');

fid = fopen('colorscale.h', 'w');
[hauteur, largeur, pro] = size(a);
counter = 0;

fprintf(fid, 'const unsigned int lookup_TFT_RGB565[256] = {');  % Utilisation de "unsigned short" pour représenter les données sur 16 bits
for i = 1:1:hauteur
    for j = 1:1:largeur
        red = a(i, j, 1);    % Composante rouge
        green = a(i, j, 2);  % Composante verte
        blue = a(i, j, 3);   % Composante bleue
        
        % Conversion vers RGB565
        red_5bit = bitshift(red, -3);       % Les 5 bits les plus significatifs pour le rouge
        green_6bit = bitshift(green, -2);   % Les 6 bits les plus significatifs pour le vert
        blue_5bit = bitshift(blue, -3);     % Les 5 bits les plus significatifs pour le bleu
        
        % Combinaison des bits
        rgb565 = bitor(bitshift(uint16(red_5bit), 11), bitshift(uint16(green_6bit), 5));  % Conversion en "uint16" pour obtenir 16 bits
        rgb565 = bitor(rgb565, uint16(blue_5bit));
        
        % Affichage du résultat
%         disp(dec2hex(rgb565, 4));
        
        counter = counter + 1;
        fprintf(fid, '0x');
        fprintf(fid, '%04X', rgb565);  % Formatage sur 4 caractères hexadécimaux (4 chiffres)
        fprintf(fid, ',');
        if rem(counter, 16) == 0
            fprintf(fid, '\n');
        end
    end
end
fseek(fid,-2,'cof');
fprintf(fid, '};');
fclose(fid);
disp('Done !')