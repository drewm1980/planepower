load DataSim
clc

figure(1);clf

tic
timedisp = toc;

 SlowMoDisp = 1;
 ax = 1.1*max(max(abs(state(:,1:3))));
 
 PatchSourceWing = [0.05 -0.05 -0.05 0.05;
                    0.5   0.5  -0.5 -0.5;
                    0    0    0     0];
                
PatchSourceTailH = 0.5*PatchSourceWing - LT*[1;0;0]*ones( 1,size(PatchSourceWing,2));

PatchSourceTailV = 0.5*[0.05 -0.05 -0.05 0.05; 
                           0     0     0    0;
                         0.5   0.5     0    0] - LT*[1;0;0]*ones( 1,size(PatchSourceWing,2));
 
                     
XAMem = [];                     
while (timedisp < P.Tf*SlowMoDisp)
    timedisp = toc;
    statedisp = linint([time state],timedisp/SlowMoDisp);
    
    delta = statedisp(16);
    

    x = statedisp(1);y = statedisp(2);z = statedisp(3);
    dx = statedisp(4);dy = statedisp(5);dz = statedisp(6);
    q0 = statedisp(7);
    q1 = statedisp(8);
    q2 = statedisp(9);
    q3 = statedisp(10);
    
    ArmKinFile
    
    XAMem = [XAMem;XA'];
    
    E = [ -q1  q0 -q3  q2;
          -q2  q3  q0 -q1;
          -q3 -q2  q1  q0];
 
    G = [-q1  q0  q3 -q2;
         -q2 -q3  q0  q1;
         -q3  q2 -q1  q0];
 
    R = E*G';
    
    
   
    Xc = [x;y;z] ;

    PatchWing = [x;y;z]*ones(1,size(PatchSourceWing,2)) + R*PatchSourceWing;
    PatchTailH = [x;y;z]*ones(1,size(PatchSourceTailH,2)) + R*PatchSourceTailH;
    PatchTailV = [x;y;z]*ones(1,size(PatchSourceTailV,2)) + R*PatchSourceTailV;
    
     figure(1);clf
     whitebg([1.0 1.0 1.0]) 
    set(gcf,'Color',[1 1 1])
  
    for k = 1:3
        quiver3(x,y,z,R(1,k),R(2,k),R(3,k),0.5,'linewidth',2);hold on
    end
    
    
    patch(PatchWing(1,:)',PatchWing(2,:)',PatchWing(3,:)','k')
    patch(PatchTailH(1,:)',PatchTailH(2,:)',PatchTailH(3,:)','k')
     patch(PatchTailV(1,:)',PatchTailV(2,:)',PatchTailV(3,:)','k')
     
    line([XA(1) Xc(1)],[XA(2) Xc(2)],[0 Xc(3)],'linewidth',1,'color','b');hold on
    line([0 XA(1)],[0 XA(2)],[0 0],'linewidth',2,'color','k');hold on

    plot3(XAMem(:,1),XAMem(:,2),0*XAMem(:,2),'color','c')
    
    plot3(x,y,z,'marker','.','markersize',15,'color','b')
    plot3(state(:,1),state(:,2),state(:,3),'linewidth',1,'color','k','linestyle','--')
    %text(x,y,z,'C.M.');%,'Horizontalalignement','left')
    %plot3(Xc(1),Xc(2),Xc(3),'marker','.','markersize',15,'color','r')
    %text(Xc(1),Xc(2),Xc(3),'Tether Anchor');%,'Horizontalalignement','left')
    %quiver3(x,y,z,dx,dy,dz,'linewidth',1,'color','m');

    

   
    xlabel('x')
    ylabel('y')
    zlabel('z')
    %view(90-delta*180/pi,4)
    %view(0,10)
    view(45,45)
    grid
    %axis equal
	axis([-ax ax -ax ax -ax ax])
    
   

    drawnow
end



