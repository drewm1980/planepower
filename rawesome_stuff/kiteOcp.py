import rawe
import casadi as C

if __name__=='__main__':
    from highwind_carousel_conf import conf
    dae = rawesome.rawe.models.carousel(conf)

    from rawe.ocp import Ocp
    mpc = Ocp(dae, N=10, ts=0.2)
    mpc.constrain(mpc['x'], '==', 3, when='AT_START')
    mpc.constrain(mpc['y'], '==', 0, when='AT_END')
    mpc.constrain(mpc['z'], '>=', 0)

    mpc.minimizeLsq(C.veccat([mpc['x'],mpc['y'],mpc['z']]))
    mpc.minimizeLsqEndTerm(C.veccat([mpc['dx']]))

    mpc.exportCode(CXX='clang++')
