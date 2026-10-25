# Maqueta visual de ST-01. Dimensiones provisionales en metros; no es un plano de fabricación.
from pathlib import Path
import math

BASE = Path(__file__).resolve().parent
materiales = {
 'caja': (.10,.13,.17), 'panel': (.025,.035,.05), 'metal': (.32,.39,.45),
 'texto': (.75,.86,.88), 'apagado': (.10,.035,.02), 'digito': (1,.32,.045),
 'rojo': (1,.07,.04), 'verde': (.08,.95,.25), 'azul': (.05,.35,1),
 'ambar': (1,.48,.02), 'blanco': (.92,.97,1), 'cian': (.05,.85,1),
 'magenta': (1,.07,.6),
}
vertices=[]; caras=[]; objetos=[]
def objeto(nombre, material, puntos, triangulos):
 offset=len(vertices); vertices.extend(puntos)
 objetos.append((nombre,material,len(caras),len(triangulos)))
 caras.extend(tuple(offset+i for i in t) for t in triangulos)

def caja(nombre,material,x,y,z,w,h,d,cara=None):
 pts=[(x+sx*w/2,y+sy*h/2,z+sz*d/2) for sx,sy,sz in
      [(-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),(-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1)]]
 if cara: pts=[mapear(cara,*p) for p in pts]
 quads=[(0,3,2,1),(4,5,6,7),(0,4,7,3),(1,2,6,5),(0,1,5,4),(3,7,6,2)]
 tris=[t for a,b,c,d in quads for t in [(a,b,c),(a,c,d)]]
 objeto(nombre,material,pts,tris)

def mapear(cara,u,v,d):
 return (u,v,.6+d) if cara=='A' else (.6+d,v,-u)

def disco(nombre,material,cara,u,v,d,r,prof=.008,n=32):
 pts=[mapear(cara,u,v,d+prof)]
 pts += [mapear(cara,u+r*math.cos(i*2*math.pi/n),v+r*math.sin(i*2*math.pi/n),d+prof) for i in range(n)]
 pts += [mapear(cara,u+r*math.cos(i*2*math.pi/n),v+r*math.sin(i*2*math.pi/n),d) for i in range(n)]
 tris=[]
 for i in range(n):
  a=1+i;b=1+(i+1)%n;c=1+n+i;e=1+n+(i+1)%n
  tris.extend([(0,a,b),(a,c,e),(a,e,b)])
 objeto(nombre,material,pts,tris)

FUENTE={
 'A':['01110','10001','10001','11111','10001','10001','10001'],
 'B':['11110','10001','10001','11110','10001','10001','11110'],
 'C':['01111','10000','10000','10000','10000','10000','01111'],
 'R':['11110','10001','10001','11110','10100','10010','10001'],
 'S':['01111','10000','10000','01110','00001','00001','11110'],
 'T':['11111','00100','00100','00100','00100','00100','00100'],
 'L':['10000','10000','10000','10000','10000','10000','11111'],
 'N':['10001','11001','11001','10101','10011','10011','10001'],
 'D':['11110','10001','10001','10001','10001','10001','11110'],
 'E':['11111','10000','10000','11110','10000','10000','11111'],
 'I':['11111','00100','00100','00100','00100','00100','11111'],
 'O':['01110','10001','10001','10001','10001','10001','01110'],
 '0':['01110','10001','10011','10101','11001','10001','01110'],
 '1':['00100','01100','00100','00100','00100','00100','01110'],
 '-':['00000','00000','00000','11111','00000','00000','00000'],
 ' ':['00000']*7,
}
def texto(cara,cadena,u,v,tam=.008):
 ancho=(len(cadena)*6-1)*tam
 for k,ch in enumerate(cadena):
  for j,linea in enumerate(FUENTE[ch]):
   for i,bit in enumerate(linea):
    if bit=='1':caja('texto_'+cara,'texto',u-ancho/2+(k*6+i+.5)*tam,v+(3-j)*tam,.015,tam*.85,tam*.85,.003,cara)

caja('envolvente_1200mm','caja',0,.68,0,1.2,1.2,1.2)
for cara in ['A','B']:
 caja('panel_cara_'+cara,'panel',0,.68,.004,1.12,1.12,.012,cara)
 texto(cara,'CARA '+cara,0,1.17,.014)
 for u in [-.52,.52]:
  for v in [.16,1.20]:disco('tornillo_'+cara,'metal',cara,u,v,.013,.013,.004,12)
# Cuatro dígitos de siete segmentos: 1985, muestra estática.
segmentos={'a':(0,.145,.115,.023),'b':(.065,.077,.023,.112),'c':(.065,-.077,.023,.112),'d':(0,-.145,.115,.023),'e':(-.065,-.077,.023,.112),'f':(-.065,.077,.023,.112),'g':(0,0,.115,.023)}
patrones={'1':'bc','9':'abcdfg','8':'abcdefg','5':'acdfg'}
for k,dig in enumerate('1985'):
 u=(k-1.5)*.235
 caja('marco_digito_'+str(k),'metal',u,.88,.016,.212,.39,.012,'A')
 caja('fondo_digito_'+str(k),'panel',u,.88,.024,.194,.37,.01,'A')
 for seg,(x,y,w,h) in segmentos.items():caja('digito_'+str(k)+'_'+seg,'digito' if seg in patrones[dig] else 'apagado',u+x,.88+y,.032,w,h,.007,'A')
 disco('punto_decimal','apagado','A',u+.09,.72,.033,.012)
# Barra RGB de Cara A.
caja('perfil_barra','metal',0,.53,.019,.93,.11,.035,'A')
for i in range(24):caja('barra_pixel_'+str(i),['cian','azul','magenta'][i//8],(i-11.5)*.036,.53,.041,.03,.06,.008,'A')
texto('A','BARRA LED',0,.425,.007)
# Cuatro pilotos de Cara A: azul, verde, naranja y rojo.
for i,color in enumerate(['azul','verde','ambar','rojo']):
 u=(i-1.5)*.235
 disco('aro_piloto_A_'+str(i),'metal','A',u,.29,.02,.063,.012)
 disco('piloto_A_'+str(i),color,'A',u,.29,.034,.049,.012)
# Tres pilotos de Cara B: naranja, blanco izquierdo y blanco derecho.
for i,color in enumerate(['ambar','blanco','blanco']):
 u=(i-1)*.30
 disco('aro_piloto_B_'+str(i),'metal','B',u,.96,.02,.068,.012)
 disco('piloto_B_'+str(i),color,'B',u,.96,.034,.053,.012)
texto('B','LNB',-.30,.84,.008)
# Tira COB en un recorrido rectangular, representada con emisores contiguos.
for i in range(24):
 u=(i-11.5)*.035
 for v in [.33,.70]:caja('tira_horizontal','cian' if v>.5 else 'ambar',u,v,.028,.032,.027,.008,'B')
for i in range(10):
 for u in [-.42,.42]:caja('tira_vertical','magenta',u,.345+i*.035,.028,.027,.032,.008,'B')
texto('B','TIRA LED',0,.515,.011)
disco('estado_dmx_azul','azul','B',.41,.20,.03,.025,.01)
texto('B','ESTADO',.23,.20,.006)
# Patas bajo el cubo.
for x in [-.45,.45]:
 for z in [-.45,.45]:caja('pata','panel',x,.04,z,.14,.08,.14)

mtl=['# Materiales de la maqueta ST-01']
for nombre,rgb in materiales.items():
 brillo=.25 if nombre in ['digito','rojo','verde','azul','ambar','blanco','cian','magenta'] else 0
 mtl += [f'newmtl {nombre}','Ka '+ ' '.join(map(str,rgb)),'Kd '+ ' '.join(map(str,rgb)),'Ks 0.15 0.15 0.15',f'Ke {rgb[0]*brillo} {rgb[1]*brillo} {rgb[2]*brillo}','Ns 24','illum 2','']
(BASE/'ST-01-cubo.mtl').write_text('\n'.join(mtl)+'\n')
obj=['# ST-01: cubo provisional de 1,2 m; Cara A frontal, Cara B derecha.','mtllib ST-01-cubo.mtl']
obj += ['v %.6f %.6f %.6f'%v for v in vertices]
# Normales explícitas para una importación uniforme en los visores OBJ.
for f in caras:
 a,b,c=[vertices[k] for k in f]
 u=[b[i]-a[i] for i in range(3)];v=[c[i]-a[i] for i in range(3)]
 n=(u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0])
 longitud=math.sqrt(sum(t*t for t in n))
 assert longitud>0
 obj.append('vn %.6f %.6f %.6f'%tuple(t/longitud for t in n))
for idx,(nombre,mat,inicio,total) in enumerate(objetos):
 obj += [f'o {nombre}_{idx}',f'usemtl {mat}','s off']
 obj += ['f '+' '.join(str(i+1)+'//'+str(inicio+j+1) for i in f) for j,f in enumerate(caras[inicio:inicio+total])]
(BASE/'ST-01-cubo.obj').write_text('\n'.join(obj)+'\n')
print(f'Modelo generado: {len(vertices)} vertices, {len(caras)} triangulos, {len(objetos)} piezas.')
