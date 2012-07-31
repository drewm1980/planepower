function  [sys,x0,str,ts]=  TAC_model_sfcn(t,x,u,flag,state)
switch flag
    case 0                         % initialize
        str=[]                     ;
        ts = [0 0]                 ;
        s = simsizes               ;
        s.NumContStates = 22       ;
        s.NumDiscStates = 0        ;
        s.NumOutputs = 22          ;
        s.NumInputs = 4            ;
        s.DirFeedthrough = 0       ;
        s.NumSampleTimes = 1       ;
        sys = simsizes(s)          ;
        x0 = state                 ;
    case 1                         % derivatives
        sys = Model_integ_ACADO_simulink(t,x,u,'');
    case 3  % output
        sys = x;
case {2 4 9}                       % 2:discrete
                                   % 4:calcTimeHit
                                   % 9:termination
        sys =[];
    otherwise
error(['unhandled flag =',num2str(flag)])   ;
end
