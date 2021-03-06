function J = invariantJac(in1)

rA = 1.085;

ddelta = in1(22);
delta = in1(21);
dr = in1(20);
dx = in1(4);
dy = in1(5);
dz = in1(6);
e11 = in1(7);
e12 = in1(8);
e13 = in1(9);
e21 = in1(10);
e22 = in1(11);
e23 = in1(12);
e31 = in1(13);
e32 = in1(14);
e33 = in1(15);
r = in1(19);
x = in1(1);
y = in1(2);
z = in1(3);
t2 = cos(delta);
t3 = sin(delta);
t4 = rA.*t3;
t5 = t4+x;
t6 = y-rA.*t2;
J = reshape([t5,dx+ddelta.*rA.*t2,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,t6,dy+ddelta.*rA.*t3,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,z,dz,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,t5,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,t6,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,z,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,e11.*2.0,e21,e31,e21,0.0,0.0,e31,0.0,0.0,0.0,0.0,e12.*2.0,e22,e32,e22,0.0,0.0,e32,0.0,0.0,0.0,0.0,e13.*2.0,e23,e33,e23,0.0,0.0,e33,0.0,0.0,0.0,0.0,0.0,e11,0.0,e11,e21.*2.0,e31,0.0,e31,0.0,0.0,0.0,0.0,e12,0.0,e12,e22.*2.0,e32,0.0,e32,0.0,0.0,0.0,0.0,e13,0.0,e13,e23.*2.0,e33,0.0,e33,0.0,0.0,0.0,0.0,0.0,e11,0.0,0.0,e21,e11,e21,e31.*2.0,0.0,0.0,0.0,0.0,e12,0.0,0.0,e22,e12,e22,e32.*2.0,0.0,0.0,0.0,0.0,e13,0.0,0.0,e23,e13,e23,e33.*2.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,-r,-dr,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,-r,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0],[11, 22]);
