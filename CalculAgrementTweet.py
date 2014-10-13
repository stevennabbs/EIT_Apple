'''
Created on 13 oct. 2014

@author: romain
'''
import re

chemin = "/home/romain/Documents/EIT/Etiquetté/"
fichier = "corpusEtq.003"
fichier2 = "steven/3_suite.txt"

file = open(chemin + fichier, mode='r')
file2 = open(chemin + fichier2, mode='r')

lines = file.readlines()

corpus1 = {}
corpus2 = {}
for l in lines:
    m = re.search('(\d+),([^,]+)', l)
    print (m.group(1) + " " + m.group(2))
    id, lbl = m.group(1), m.group(2)
    corpus1[id] = lbl

lines = file2.readlines()    
for l in lines:
    m = re.search('(\d+),([^,]+)', l)
    id, lbl = m.group(1), m.group(2)
    corpus2[id] = lbl
    
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

file.close()

def calculAgrement():
    return 0
    
def calculTweetPareil():
    return 0.0
        


