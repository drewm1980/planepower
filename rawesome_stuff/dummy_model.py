
import rawe

def dummyModel():
    dae = rawe.models.Dae()
    dae.addX(['x',
              'y',
              'z',
              'dx',
              'dy',
              'dz',
              'e11',
              'e12',
              'e13',
              'e21',
              'e22',
              'e23',
              'e31',
              'e32',
              'e33',
              'wx',
              'wy',
              'wz',
              'delta',
              'ddelta',
              'ur',
              'up'])
    dae.addU(['dddelta',
              'dur',
              'dup'])
    
    return dae
