'''
Created on 13 oct. 2014

@author: romain
'''
import re
from sys import argv


s , fichier1, fichier2 = argv
print('%s, %s, %s' % (s, fichier1, fichier2))

try:
    file = open(fichier1, mode='r')
    file2 = open(fichier2, mode='r')
except:
    print("Erreur sur l'ouverture des fichiers")
    exit
    
    
lines = file.readlines()

corpus1 = {}
corpus2 = {}
for l in lines:
    m = re.search('(\d+),([^,]+)', l)
    id, lbl = m.group(1), m.group(2)
    corpus1[id] = lbl

lines = file2.readlines()    
for l in lines:
    m = re.search('(\d+),([^,]+)', l)
    id, lbl = m.group(1), m.group(2)
    corpus2[id] = lbl

file.close()
file2.close()
    
mat = {'pos':{'pos':0, 'neu':0, 'neg':0, 'irr':0},
     'neu': {'pos':0, 'neu':0, 'neg':0, 'irr':0},
      'neg':{'pos':0, 'neu':0, 'neg':0, 'irr':0},
      'irr':{'pos':0, 'neu':0, 'neg':0, 'irr':0}}          

cnt = 0
for id, lbl1 in corpus1.items():
    if corpus2[id]:
        lbl2 = corpus2[id]
        mat[lbl1][lbl2] +=  1 
        cnt = cnt +1

lbls = {"pos", "neu", "neg", "irr"}

for e in lbls:
    for i in lbls:
        print("[%s][%s][%d]" % (e, i, mat[e][i]), end='\t')
    print()        
        

ok = mat['pos']['pos'] + mat['neg']['neg'] + mat['neu']['neu'] + mat['irr']['irr']
print("tweet pareil : %f %%" % (ok / 100.0))

p0 = ok / cnt

pe = 0

        

for e in lbls:
    n_pi, n_ip = 0, 0
    for i in lbls:
        n_pi = n_pi + mat[i][e]
        n_ip = n_ip + mat[e][i]            
    
    pe += n_pi * n_ip      
  
pe /= (cnt * cnt)
kappa = (p0 - pe) / (1-pe)
print("agrément : %f" %  kappa)



def calculAgrement():
    return 0
    
def calculTweetPareil():
    return 0.0
        


