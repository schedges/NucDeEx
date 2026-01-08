import os

nakazato_Ex_16F_MeV = [0.25, 0.30, 0.34, 3.76, 4.51, 4.52, 5.66, 5.78, 6.71, 7.03,
                      7.57, 8.06, 8.37, 9.52, 9.95, 10.61, 10.81, 11.69, 11.82, 12.31,
                      12.35, 12.51, 12.67, 12.90, 12.92, 13.08, 13.18, 13.20, 13.24, 13.61,
                      14.02, 14.29, 14.45, 15.06, 15.07, 15.34, 15.90, 16.18, 16.79, 16.85,
                      18.08]

codePath = "bin/simulation_nakazato_16O_CC"
nps = 10000
os.environ["NUCDEEX_ROOT"] = os.getcwd()

for nrg in nakazato_Ex_16F_MeV:
    cmd = f"{codePath} {nrg:.3f} {nps}"
    os.system(cmd)