#!/bin/python
import numpy as np
import netCDF4
from netCDF4 import Dataset
import xarray
from scipy import signal
from scipy.ndimage.filters import convolve
from scipy.misc import imread, imshow
import matplotlib.pyplot as plt

'''
Parametros
'''
w = np.array([[-1,-1,-1],[-1,8,-1],[-1,-1,-1]]) #MATRIZ CON LA QUE SE REALIZA LA CONVOLUCION
print w

dataDIR = "/home/nestormann/Documents/SOII/2019/TP2misc/asd.nc"

'''
Abrir el dataset como una matriz XARRAY y guardar la matriz CMI
'''
DS = Dataset(dataDIR)
f = DS.variables['CMI']
print f.shape

h = f[2180:2185,2180:2185]
print h.dtype
print h
# plt.imshow(h)
# plt.show()
'''
kleiner = np.array([[10,2,2,2,2],[2,10,2,2,2],[2,2,10,2,2],[2,2,2,10,2],[2,2,2,2,10]])
print (kleiner)

Convolucion

g = signal.convolve2d(kleiner,w,boundary='fill',mode='same')
h = convolve(kleiner,w)
y = signal.convolve2d(kleiner,w,boundary='symm',mode='valid')
print(g)
print g.shape
print(h)
print h.shape
print(y)
print y.shape

y = g[10000:10200,10000:10200]
print y.shape
plt.imshow(y)
plt.show()'''
