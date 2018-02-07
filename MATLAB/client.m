t = tcpip('192.168.200.1', 9001, 'NetworkRole', 'client'); %criando o objeto tcpip com os parametros corretos para a comunicação com o rasp
fopen(t);%abrindo o  canal de comunicação

while true
    
    data = fread(t, 60);
    fwrite(t, "ok!");
    tdata = char(data');
    
    datanum = regexp(tdata,'[+-]?\d+\.?\d*', 'match');
    
    acellX = str2num(datanum{1});
    acellY = str2num(datanum{2});
    acellZ = str2num(datanum{3});
    magX = str2num(datanum{4});
    magY = str2num(datanum{5});
    magZ = str2num(datanum{6});
    
    normacell = sqrt(acellX^2 + acellY^2 +acellZ^2);
    normamag = sqrt(magX^2 + magY^2 +magZ^2);
    
    acellX = acellX/normacell;
    acellY = acellY/normacell;
    acellZ = acellZ/normacell;
    magX = magX/normamag;
    magY = magY/normamag;
    magZ = magZ/normamag;
    
    acelvec = [acellX acellY acellZ];
    %magvec = [magX magY magZ];
    
    o = [0 0 0];  %# Origin
    
    %starts = zeros(2,3);
    %ends = [acelvec;magvec];

    %quiver3(starts(:,1), ends(:,1),starts(:,2), ends(:,2) )
    %axis equal
    
    
    
    plot3(acellX,acellY,acellZ,'bo')
    hold on
    plot3(acellX/2,acellY/2,acellZ/2,'bo')
    plot3(acellX/4,acellY/4,acellZ/4,'bo')
    plot3(3*acellX/4,3*acellY/4,3*acellZ/4,'bo')
    
    
    plot3(magX,magY,magZ,'ro')
    plot3(magX/2,magY/2,magZ/2,'ro')
    plot3(magX/4,magY/4,magZ/4,'ro')
    plot3(3*magX/4,3*magY/4,3*magZ/4,'ro')
    
    
    plot3(0,0,0,'blacko')
    xlim([-1 1])
    ylim([-1 1])
    zlim([-1 1])
    grid on
    hold off
    %vectarrow(o,acelvec)
    
    pause(0.05)
end

fclose(t);%fechando o canal de comunicação