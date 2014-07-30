function res = diffDepVar(fun,depVar)
syms xx
res = diff(subs(fun,depVar,xx),xx);
res = subs(res,xx,depVar);
end
