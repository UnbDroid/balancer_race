function [outputArg1,outputArg2] = plotvctors(inputArg1,inputArg2)
%PLOTVCTORS Summary of this function goes here
%   Detailed explanation goes here

%código que rodava anteriormente no script client.m
%colando ele aqui sem mudanças apenas para não perder as informações
%mudanças ainda devem ser feitas para usar este código como função
acellX = acellX/normacell;
    normacell = sqrt(acellX^2 + acellY^2 +acellZ^2);
    normamag = sqrt(magX^2 + magY^2 +magZ^2);


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


outputArg1 = inputArg1;
outputArg2 = inputArg2;
end

