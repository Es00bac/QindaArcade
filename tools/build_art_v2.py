"""Blender MCP authoring: eight character-built karts and twenty working-world props.

Run in tools/blender_bootstrap.py's isolated factory-startup session. QMS2 stores
per-face material parameters as well as metric UVs; decals have separate islands.
"""
import bpy
import math
import json
from pathlib import Path
from mathutils import Vector

ROOT = Path('/home/cabewse/work_SPaC3/Games/QindaMegaBrawlSmash')
OUT = ROOT / 'assets/models'
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
groups = {}
def xyz(p): return (p[0], -p[2], p[1])
def mat(name, rgb, rough=.5, metal=0, mode=0):
    m = bpy.data.materials.new(name)
    m.diffuse_color = (*rgb, 1)
    m.use_nodes = True
    s = m.node_tree.nodes.get('Principled BSDF')
    s.inputs['Base Color'].default_value = (*rgb, 1)
    s.inputs['Roughness'].default_value = rough
    s.inputs['Metallic'].default_value = metal
    m['runtime'] = [*rgb, rough, metal, mode]
    return m

steel = mat('Brushed exposed steel', (.34,.39,.41), .42, .78, 30)
dark = mat('Powder-coated frame', (.065,.082,.088), .66, .3, 28)
rubber = mat('Rubber / cable insulation', (.045,.052,.052), .88, 0, 27)
canvas = mat('Worn canvas tool roll', (.38,.32,.20), .92, 0, 27)
ivory = mat('Warm ivory enamel', (.82,.80,.66), .5, .18, 28)
orange = mat('Safety orange enamel', (.91,.32,.09), .57, .22, 28)
yellow = mat('Machine yellow enamel', (.85,.61,.12), .56, .25, 28)
copper = mat('Copper plumbing', (.62,.31,.15), .39, .74, 30)
glass = mat('Blue-black glass', (.05,.13,.16), .18, .3)
water = mat('Reservoir blue', (.20,.52,.60), .28, .2)
wood = mat('Oiled reclaimed timber', (.40,.34,.26), .76, 0, 29)
green = mat('Garden service green', (.25,.39,.21), .66, .1, 28)
leaf = mat('Matte alpine needles', (.16,.25,.18), .95, 0)
snow = mat('Settled snow', (.8,.85,.85), .93, 0)
pcb = mat('Exposed circuit substrate', (.21,.35,.23), .74, .12, 22)
colors = [(0.12,.53,.43),(.89,.61,.14),(.73,.11,.16),(.16,.38,.58),(.51,.38,.72),(.07,.075,.12),(.38,.47,.20),(.82,.38,.44)]
paints = [mat('Kart %02d / primary paint'%i,c,.39 if i==5 else .53,.32,31 if i==5 else 28) for i,c in enumerate(colors)]
pink = mat('Vanta magenta accent', (.86,.12,.51), .31, .5, 31)
mint = mat('Rill seafoam enamel', (.38,.68,.61), .55, .16, 28)

def add(obj, group, material, label):
    obj.name = group + ' / ' + label
    obj.data.materials.append(material)
    groups.setdefault(group, []).append(obj)
    return obj

def bevel(obj, width):
    if width:
        b=obj.modifiers.new('Physical panel edge', 'BEVEL'); b.width=width; b.segments=2
        obj.modifiers.new('Weighted manufactured normals', 'WEIGHTED_NORMAL')

def box(g,p,h,m=steel,b=.025,label='Panel'):
    bpy.ops.mesh.primitive_cube_add(size=2, location=xyz(p))
    o=bpy.context.object; o.scale=(h[0],h[2],h[1])
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    bevel(o,b);return add(o,g,m,label)

def ell(g,p,h,m,label='Formed pod'):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=24,ring_count=12,location=xyz(p))
    o=bpy.context.object;o.scale=(h[0],h[2],h[1])
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    for face in o.data.polygons: face.use_smooth=True
    return add(o,g,m,label)

def tube(g,a,b,r,m=steel,label='Tube',verts=16):
    a,b=Vector(xyz(a)),Vector(xyz(b));d=b-a
    bpy.ops.mesh.primitive_cylinder_add(vertices=verts,radius=r,depth=d.length,location=(a+b)*.5)
    o=bpy.context.object;o.rotation_euler=d.to_track_quat('Z','Y').to_euler()
    bevel(o,min(r*.2,.016));return add(o,g,m,label)

def ring(g,p,r,thick,m=steel,axis='y',label='Band'):
    bpy.ops.mesh.primitive_torus_add(major_segments=24,minor_segments=6,major_radius=r,minor_radius=thick,location=xyz(p))
    o=bpy.context.object
    if axis=='x': o.rotation_euler[1]=math.pi/2
    if axis=='z': o.rotation_euler[0]=math.pi/2
    return add(o,g,m,label)

def panel(g,points,depth,m,label='Folded panel'):
    # Points are a top-view outline (x,y,z), with a second skin below it.
    n=len(points);v=[xyz(p) for p in points]+[xyz((p[0],p[1]-depth,p[2])) for p in points]
    faces=[tuple(range(n)),tuple(reversed(range(n,2*n)))]
    faces += [(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
    mesh=bpy.data.meshes.new(label);mesh.from_pydata(v,[],faces);mesh.update()
    o=bpy.data.objects.new(label,mesh);bpy.context.collection.objects.link(o);bevel(o,min(.027,depth*.35))
    return add(o,g,m,label)

def decal(g,id,col,p,w,h,axis='front',label='Sponsor island',slope=0,curve=None):
    # 2:1 signs, isolated from paint UVs. Separate faces prevent edge wrapping.
    x,y,z=p
    if axis=='top': pts=[(x-w/2,y,z+h/2),(x+w/2,y,z+h/2),(x+w/2,y,z-h/2),(x-w/2,y,z-h/2)]
    elif axis=='side': pts=[(x,y-h/2,z+w/2),(x,y-h/2,z-w/2),(x,y+h/2,z-w/2),(x,y+h/2,z+w/2)]
    elif axis=='back': pts=[(x+w/2,y-h/2,z),(x-w/2,y-h/2,z),(x-w/2,y+h/2,z),(x+w/2,y+h/2,z)]
    else: pts=[(x-w/2,y-h/2,z),(x+w/2,y-h/2,z),(x+w/2,y+h/2,z),(x-w/2,y+h/2,z)]
    # A grid follows the actual curved shell, so the graphic neither floats nor clips.
    points=[];faces=[];coords=[];nu,nv=(12,6) if curve else (1,1)
    for v in range(nv+1):
        for u in range(nu+1):
            a,b=u/nu,v/nv
            q=Vector(pts[0])*(1-a)*(1-b)+Vector(pts[1])*a*(1-b)+Vector(pts[2])*a*b+Vector(pts[3])*(1-a)*b
            if axis=='top':q.y+=slope*(q.z-z)
            if curve:
                cx,cy,cz,rx,ry,rz=curve
                if axis=='top':q.y=cy+ry*math.sqrt(max(.01,1-((q.x-cx)/rx)**2-((q.z-cz)/rz)**2))+.006
                elif axis=='front':q.z=cz+rz*math.sqrt(max(.01,1-((q.x-cx)/rx)**2-((q.y-cy)/ry)**2))+.006
            points.append(xyz(q));coords.append(((col+.025+.95*a)/4,.025+.95*b))
    for v in range(nv):
        for u in range(nu):
            k=v*(nu+1)+u;faces.append((k,k+1,k+nu+2,k+nu+1))
    mesh=bpy.data.meshes.new(label);mesh.from_pydata(points,[],faces)
    uv=mesh.uv_layers.new(name='Decal islands / column %d'%col)
    for loop in mesh.loops:uv.data[loop.index].uv=coords[loop.vertex_index]
    o=bpy.data.objects.new(label,mesh);bpy.context.collection.objects.link(o);o['decal']=True
    m=mat('Livery %d / %s'%(id,label),(1,1,1),.65,0,16+id*.1)
    add(o,g,m,label)

def rivets(g,x,y,z,n=5,step=.18,axis='x'):
    for i in range(n):
        p=[x,y,z];p[0 if axis=='x' else 2]+=i*step
        ell(g,p,(.022,.016,.022),steel,'Panel rivet')

def crate(g,p,size=(1,1,1),m=wood):
    x,y,z=p;sx,sy,sz=size
    box(g,(x,y+sy*.5,z),(sx*.5,sy*.5,sz*.5),m,.04,'Crate body')
    for s in [-1,1]:
        box(g,(x+s*sx*.38,y+sy*.5,z+sz*.51),(.065,sy*.48,.04),steel)
        box(g,(x+s*sx*.38,y+sy*.5,z-sz*.51),(.065,sy*.48,.04),steel)

for i in range(8):
    g='kart-'+str(i);p=paints[i]
    # A shared homologated floor and mounting points, not a shared visible body.
    box(g,(0,.54,0),(.91,.10,1.42),dark,.055,'Homologated floor')
    for s in [-1,1]:
        tube(g,(s*.8,.55,-1.42),(s*.8,.55,1.37),.056,dark,'Chassis rail')
        box(g,(s*.65,.62,-1.59),(.13,.055,.035),orange,.015,'Tail reflector')
    if i==0:
        panel(g,[(-.88,.95,.58),(.88,.95,.58),(.74,.78,1.63),(-.74,.78,1.63)],.24,p,'Rescue wedge bonnet')
        box(g,(0,.685,1.64),(.84,.09,.055),ivory)
        for s in [-1,1]:
            box(g,(s*.9,.81,-.12),(.12,.22,.65),p)
            tube(g,(s*.96,.76,-1.2),(s*.96,1.09,-.55),.038,steel)
            box(g,(s*.61,.88,-1.26),(.25,.29,.28),dark,.055,'Removable battery')
            box(g,(s*.61,.90,-1.56),(.22,.19,.015),p)
            tube(g,(s*.4,.93,1.62),(s*.75,.93,1.62),.033,steel,'Headlight guard')
        ring(g,(0,.61,1.77),.13,.035,orange,'z','Recovery eye')
        box(g,(0,1.56,-.83),(.58,.07,.08),steel)
        for s in [-1,1]:tube(g,(s*.53,.70,-.83),(s*.53,1.56,-.83),.035,steel,'Rescue roll bar upright')
        ell(g,(0,1.68,-.83),(.12,.13,.12),orange,'Amber rescue beacon')
        decal(g,i,0,(0,.922,.80),.74,.36,'top',slope=-.1619)
        decal(g,i,1,(1.024,.81,-.12),1.0,.36,'side')
        for s in [-1,1]:box(g,(s*.60,.76,1.65),(.17,.068,.025),ivory,.02,'Protected rescue headlamp')
    elif i==1:
        ell(g,(0,.71,.60),(.88,.37,1.09),p,'Pressed laundry tub')
        box(g,(0,.97,.19),(.55,.14,.50),dark,.15,'Tub cockpit liner')
        tube(g,(-.60,.99,-1.15),(.60,.99,-1.15),.33,copper,'Pressure accumulator',24)
        for x in [-.45,0,.45]: ring(g,(x,.99,-1.15),.335,.028,steel,'x')
        for s in [-1,1]:
            ell(g,(s*1.11,.89,1.02),(.35,.16,.61),p,'Scalloped mudguard')
            tube(g,(s*.80,.67,1.02),(s*1.10,.84,1.02),.028,dark,'Mudguard bracket')
            tube(g,(s*.62,.97,-1.20),(s*.62,1.65,-1.30),.075,copper,'Short exhaust stack')
            ring(g,(s*.62,1.62,-1.30),.095,.025,dark)
            rivets(g,s*.78,.86,.45,6,.16,'z')
        ring(g,(.8,1.11,-.95),.16,.025,orange,'x','Manual shutoff')
        ell(g,(-.61,1.37,-1.10),(.12,.12,.035),ivory,'Pressure dial')
        decal(g,i,0,(0,.77,1.51),.58,.28,curve=(0,.71,.60,.88,.37,1.09))
        decal(g,i,1,(0,1.083,.86),.72,.35,'top',curve=(0,.71,.60,.88,.37,1.09))
    elif i==2:
        panel(g,[(-.95,.99,.36),(.94,.99,.36),(.75,.78,1.65),(-.75,.78,1.65)],.20,p,'Chopped rally bonnet')
        panel(g,[(-1.13,.51,.95),(1.13,.51,.95),(1.08,.51,1.78),(-1.08,.51,1.78)],.08,dark,'Rally splitter')
        for s in [-1,1]:
            panel(g,[(s*.72,.98,.7),(s*1.04,.82,.42),(s*1.02,.88,-.75),(s*.68,1.05,-.70)],.2,p if s>0 else dark,'Replaced side flare')
            tube(g,(s*.73,.68,-1.34),(s*.73,1.63,-1.34),.035,steel,'Wing stay')
        box(g,(0,1.64,-1.40),(1.13,.05,.26),p)
        for s in [-1,1]:box(g,(s*1.13,1.68,-1.4),(.036,.14,.28),dark)
        box(g,(.33,1.035,.69),(.18,.07,.26),dark,.015,'Offset air intake')
        box(g,(0,.86,-1.39),(.53,.22,.21),steel,.015,'Rear intercooler')
        for x in range(10):box(g,(-.46+x*.102,.87,-1.61),(.022,.17,.012),dark,.003)
        decal(g,i,0,(-.25,.957,.61),.64,.30,'top',slope=-.1628)
        decal(g,i,1,(0,1.70,-1.4),1.10,.45,'top')
        decal(g,i,3,(1.063,.80,-.01),.87,.4,'side','Hand-sprayed X')
    elif i==3:
        box(g,(0,.82,.95),(.84,.16,.68),p,.045,'Salvaged road-sign bonnet')
        box(g,(-.72,.87,.25),(.18,.23,.31),ivory,.025,'Replacement cream panel')
        box(g,(0,.76,-1.12),(.90,.09,.50),wood,.015,'Cargo bed')
        for s in [-1,1]:
            for z in [-1.55,-.72]:tube(g,(s*.89,.8,z),(s*.89,1.35,z),.043,steel)
            tube(g,(s*.89,1.35,-1.55),(s*.89,1.35,-.72),.043,steel)
        box(g,(-.51,1.03,-1.13),(.30,.2,.29),orange,.035,'Borrowed tool chest')
        crate(g,(.46,.85,-1.18),(.5,.62,.56),ivory)
        for z in [-1.36,-1.02]:box(g,(.46,1.48,z),(.27,.018,.035),dark)
        tube(g,(-.89,1.30,-1.43),(-.94,2.03,-1.44),.011,steel,'Courier aerial')
        ring(g,(-.99,1.09,-.24),.26,.065,rubber,'x','Recovered spare tire')
        tube(g,(-1.01,1.09,-.24),(-1.09,1.09,-.24),.16,steel,'Spare tire hub')
        box(g,(.69,1.01,.91),(.15,.018,.39),steel,.01,'Bolted seam repair')
        rivets(g,.61,1.034,.64,4,.17,'z')
        decal(g,i,0,(-.25,.986,.94),.80,.36,'top')
        decal(g,i,1,(0,.83,1.64),1.10,.34)
    elif i==4:
        panel(g,[(-.42,1.02,.26),(.42,1.02,.26),(.25,.71,1.76),(-.25,.71,1.76)],.17,p,'Hand-built hillclimb nose')
        for s in [-1,1]:
            panel(g,[(s*.69,.67,-.62),(s*1.02,.67,-.82),(s*.98,.63,.69),(s*.66,.75,.38)],.12,ivory,'Open aero outrigger')
            tube(g,(s*.38,.71,1.36),(s*.93,.61,.4),.033,steel,'Exposed triangulation')
            tube(g,(s*.38,.71,.43),(s*.95,.66,.77),.026,steel)
        box(g,(0,.78,-1.18),(.45,.22,.36),steel,.03,'Compact electric transaxle')
        for x in range(9):box(g,(-.4+x*.10,1.02,-1.18),(.02,.08,.31),dark,.008)
        box(g,(0,.68,1.64),(.97,.035,.16),ivory)
        box(g,(0,1.30,-1.43),(.91,.04,.19),p)
        for s in [-1,1]:tube(g,(s*.42,.88,-1.2),(s*.42,1.3,-1.4),.025,steel)
        decal(g,i,0,(0,.918,.79),.44,.24,'top',slope=-.20667)
        decal(g,i,1,(0,1.345,-1.43),.65,.28,'top')
    elif i==5:
        for s in [-1,1]:
            panel(g,[(s*.18,.71,1.68),(s*.92,.80,1.40),(s*.80,1.04,.38),(s*.30,1.00,.51)],.21,p,'Forked stealth bow')
            panel(g,[(s*.64,.99,.35),(s*1.01,.87,.14),(s*.96,1.05,-1.18),(s*.68,1.21,-1.30)],.27,p,'Closed battery fairing')
            panel(g,[(s*.2,.747,1.70),(s*.91,.83,1.41),(s*.82,.87,1.25),(s*.24,.79,1.49)],.025,pink,'Magenta signature blade')
            box(g,(s*.89,.51,-1.32),(.1,.04,.35),pink)
        for x in [-.60,-.3,0,.3,.6]:box(g,(x,.52,-1.53),(.035,.14,.26),dark,.01,'Diffuser strake')
        box(g,(0,1.26,-1.42),(1.02,.045,.29),p)
        box(g,(-.64,1.075,-1.06),(.20,.01,.17),steel,.018,'Unadvertised crash repair')
        decal(g,i,0,(0,.69,1.38),.44,.22)
        decal(g,i,1,(0,1.31,-1.42),1.13,.43,'top')
    elif i==6:
        box(g,(0,.88,1.02),(.67,.26,.52),p,.10,'Field-service bonnet')
        box(g,(0,.88,1.554),(.49,.18,.02),dark,.025,'Radiator inset')
        for x in range(8):box(g,(-.43+x*.122,.89,1.58),(.019,.16,.01),steel,.003)
        box(g,(.54,1.16,.81),(.13,.015,.3),orange,.01,'Donor bonnet patch')
        for s in [-1,1]:
            box(g,(s*.85,.79,.04),(.16,.17,.60),ivory if s<0 else p)
            for y in [.84,1.03]:box(g,(s*.91,y,-.94),(.055,.07,.63),wood,.012,'Cargo bed slat')
            tube(g,(s*.94,.7,-1.47),(s*.94,1.22,-1.47),.04,steel)
            tube(g,(s*.91,.60,1.58),(s*.91,.82,1.61),.06,steel)
        tube(g,(-.98,.65,1.70),(.98,.65,1.70),.06,steel,'Welded farm bumper')
        box(g,(-.56,.99,-1.25),(.24,.32,.23),mint,.09,'Water can')
        tube(g,(.24,.99,-1.23),(.76,.99,-1.23),.17,canvas,'Canvas tool roll')
        for x in [.35,.65]:ring(g,(x,.99,-1.23),.173,.015,dark,'x','Tool roll strap')
        decal(g,i,0,(0,1.147,.98),.68,.33,'top')
        decal(g,i,1,(1.018,.80,.07),.90,.34,'side')
    else:
        ell(g,(0,.74,.84),(.83,.30,.89),p,'Converted research skiff bow')
        for s in [-1,1]:
            ell(g,(s*.82,.79,-.28),(.25,.28,1.09),mint,'Cooling sponson')
            for z in [-.81,-.1,.54]:ring(g,(s*.98,.85,z),.115,.027,ivory,'x','Observation port')
            tube(g,(s*.76,1.02,-.62),(s*.47,1.18,-1.21),.052,rubber,'Coolant hose')
        tube(g,(-.46,1.08,-1.20),(.46,1.08,-1.20),.28,water,'Sample reservoir',24)
        for x in [-.36,.36]:ring(g,(x,1.08,-1.20),.29,.026,ivory,'x')
        tube(g,(.75,.92,-1.34),(.75,1.77,-1.34),.026,steel,'Research instrument mast')
        box(g,(.75,1.73,-1.34),(.15,.1,.085),ivory)
        decal(g,i,0,(0,1.045,.72),.62,.29,'top',curve=(0,.74,.84,.83,.30,.89))
        decal(g,i,1,(0,.82,1.62),.72,.24,curve=(0,.74,.84,.83,.30,.89))
        decal(g,i,3,(1.085,.79,-.08),.71,.30,'side','Wave emblem')
    # Each machine carries a sponsor plate, a personal bumper sticker and a number.
    box(g,(0,.71,-1.675),(.60,.135,.023),dark,.016,'Sticker backing')
    decal(g,i,2,(0,.71,-1.701),1.15,.24,'back','Personal bumper sticker')
    if i==0:decal(g,i,3,(.53,.887,1.02),.31,.22,'top','Rescue cross stencil',slope=-.1619)
    if i==1:decal(g,i,3,(-.43,1.02,.89),.28,.18,'top','Pressure stencil',curve=(0,.71,.60,.88,.37,1.09))
    if i==3:decal(g,i,3,(.38,.987,1.14),.34,.17,'top','Shipping stencil')
    if i==4:decal(g,i,3,(0,.833,1.2),.25,.12,'top','Calibration marks',slope=-.20667)
    if i==6:decal(g,i,3,(-.43,1.146,.77),.27,.18,'top','Union flower stencil')
    if i==2:
        panel(g,[(-.63,.973,.51),(-.49,.973,.51),(-.38,.808,1.52),(-.53,.808,1.52)],.003,ivory,'Hand-masked rally slash')
        panel(g,[(-.40,.973,.51),(-.34,.973,.51),(-.22,.808,1.52),(-.29,.808,1.52)],.003,ivory,'Secondary rally slash')


# BLISS: sorting equipment and recognizable scrap, with paint/plastic/steel separated.
g='salvage-bin'
box(g,(0,.15,0),(2.2,.15,1.25),dark)
for s in [-1,1]:
    box(g,(s*2.12,.85,0),(.10,.7,1.25),orange)
    box(g,(0,.85,s*1.18),(2.2,.7,.09),orange)
    for x in [-1.6,-.8,0,.8,1.6]:box(g,(x,.8,s*1.28),(.07,.65,.055),steel)
for j in range(7):box(g,(-1.5+j*.49,.45+(j%3)*.23,0),(.32,.25,.75),pcb if j%2 else ivory)
g='cable-reel'
tube(g,(-.66,1.18,0),(.66,1.18,0),.76,rubber,verts=24)
for s in [-1,1]:
    tube(g,(s*.64,1.18,0),(s*.76,1.18,0),1.15,wood,'Timber spool flange',24)
    ring(g,(s*.785,1.18,0),.76,.025,steel,'x')
    ring(g,(s*.79,1.18,0),.18,.045,steel,'x')
for j in range(9):ring(g,(-.55+j*.135,1.18,0),.79,.025,dark,'x')
g='appliance-pile'
for j,(x,z) in enumerate([(-.85,-.4),(.65,.25),(-.60,1.05)]):
    box(g,(x,.75,z),(.64,.75,.59),ivory if j%2 else mint,.07,'Discarded appliance')
    ring(g,(x,.76,z+.605),.43,.065,steel,'z','Washer door')
    tube(g,(x,.76,z+.59),(x,.76,z+.66),.35,glass,verts=24)
    box(g,(x,1.34,z+.605),(.51,.075,.035),dark)
box(g,(.3,1.9,-.45),(.69,.32,.53),yellow,.08,'Broken printer')
for j in range(5):box(g,(.3,1.76+j*.045,.091),(.55,.012,.01),dark,.003)
g='compactor'
box(g,(0,.3,0),(2.6,.3,1.9),dark)
for s in [-1,1]:
    box(g,(s*2.18,2,0),(.30,1.7,1.55),yellow)
    tube(g,(s*1.7,1.4,-1.1),(s*1.7,3.95,-1.1),.16,steel,'Hydraulic ram')
box(g,(0,4.1,0),(2.5,.25,1.75),yellow)
box(g,(0,1.3,0),(1.75,.18,1.4),steel)
crate(g,(0,.6,0),(2.6,.5,2.1),dark)
box(g,(2.57,2.0,1.1),(.12,.38,.28),mint)
g='conveyor'
for s in [-1,1]:
    tube(g,(s*1.3,.1,-2.8),(s*1.3,2.0,-2.8),.1,steel)
    tube(g,(s*1.3,.1,2.8),(s*1.3,.75,2.8),.1,steel)
    tube(g,(s*1.35,2.2,-3),(s*1.35,.95,3),.12,yellow)
for j in range(17):
    z=-2.8+j*.35;y=1.57-z*.208
    tube(g,(-1.2,y,z),(1.2,y,z),.13,rubber,'Sorting roller')
box(g,(0,1.7,-.9),(.5,.16,.4),pcb)
g='scrap-car'
box(g,(0,.7,0),(1.45,.35,2.35),paints[3],.20,'Stripped donor chassis')
box(g,(0,1.35,-.25),(1.15,.38,1.15),ivory,.12,'Bare cabin')
box(g,(0,1.48,.92),(.97,.22,.024),glass)
for s in [-1,1]:
    for z in [-1.43,1.43]:tube(g,(s*1.45,.55,z),(s*1.62,.55,z),.40,steel,'Exposed wheel hub')
box(g,(0,.81,2.37),(1.22,.10,.05),steel)
g='pallet'
for z in [-.75,0,.75]:box(g,(0,.16,z),(1.2,.16,.11),wood,.015)
for x in range(6):box(g,(-1+x*.4,.36,0),(.15,.055,1.0),wood,.01)

# REACTOR: cultivation, water and power, not generic science-fiction clutter.
g='solar-array'
for s in [-1,1]:tube(g,(s*1.8,0,0),(s*1.8,2.7,-.3),.13,steel)
box(g,(0,2.85,0),(2.9,.12,1.65),steel)
for x in range(6):
    for z in range(4):box(g,(-2.43+x*.97,2.99,-1.22+z*.81),(.45,.018,.36),glass,.008,'Solar cell')
g='greenhouse'
box(g,(0,.18,0),(3.0,.18,5),wood)
for z in [-4.8,-2.4,0,2.4,4.8]:
    for s in [-1,1]:
        tube(g,(s*2.9,.3,z),(s*2.9,3,z),.065,ivory)
        tube(g,(s*2.9,3,z),(0,4.3,z),.065,ivory)
for x,y in [(-2.9,3),(0,4.3),(2.9,3)]:tube(g,(x,y,-4.8),(x,y,4.8),.07,ivory)
for s in [-1,1]:
    panel(g,[(0,4.28,-4.7),(0,4.28,4.7),(s*2.87,2.97,4.7),(s*2.87,2.97,-4.7)],.035,water,'Translucent blue roof')
    box(g,(s*1.65,.9,0),(.72,.12,4.4),wood)
    for z in [-3.6,-2.4,-1.2,0,1.2,2.4,3.6]:ell(g,(s*1.65,1.3,z),(.65,.48,.5),green,'Cultivation row')
g='water-tank'
tube(g,(0,.4,0),(0,3.5,0),1.25,mint,'Vertical storage tank',24)
for y in [.6,1.5,2.8,3.4]:ring(g,(0,y,0),1.27,.045,steel)
ell(g,(0,3.5,0),(1.25,.27,1.25),mint)
tube(g,(0,.4,1.0),(0,.4,2.3),.14,copper)
ring(g,(0,.55,1.85),.23,.03,orange)
g='pump-skid'
box(g,(0,.15,0),(1.8,.15,1.25),dark)
tube(g,(-1.1,.7,0),(.50,.7,0),.44,green,'Pump motor',24)
for x in [-.9,-.65,-.4,-.15,.1]:ring(g,(x,.7,0),.45,.025,steel,'x')
tube(g,(.65,.7,0),(1.5,.7,0),.17,copper)
tube(g,(1.5,.7,0),(1.5,1.45,0),.17,copper)
box(g,(-.75,1.4,-.6),(.45,.38,.25),ivory)
g='planter'
box(g,(0,.44,0),(2,.44,1),wood)
box(g,(0,.89,0),(1.84,.02,.84),dark,.005,'Soil bed')
for x in [-1.4,-.7,0,.7,1.4]:
    for z in [-.45,.45]:ell(g,(x,1.09,z),(.32,.3,.3),green,'Leafy crop')

# AFTERGLOW: roof services, freight and elevated transit.
g='vent-bank'
box(g,(0,1,0),(2.1,1,1.3),ivory,.09)
for x in [-1,1]:
    tube(g,(x,2,0),(x,2.08,0),.83,dark,verts=24)
    for j in range(5):
        a=j*math.tau/5;tube(g,(x,2.12,0),(x+math.cos(a)*.73,2.12,math.sin(a)*.73),.09,steel,'Fan blade')
for j in range(10):box(g,(-1.8+j*.4,1,1.315),(.085,.75,.018),dark,.007)
g='transit-shelter'
box(g,(0,.15,0),(4.2,.15,1.8),steel)
for s in [-1,1]:tube(g,(s*3.9,0,-1.3),(s*3.9,3.3,-1.3),.095,steel)
box(g,(0,3.35,0),(4.3,.12,1.85),paints[3])
box(g,(0,1.9,-1.34),(3.82,1.3,.035),glass)
box(g,(0,.9,-.55),(3.1,.09,.44),wood)
box(g,(3.1,2,1.4),(.44,.85,.10),dark)
box(g,(3.1,2.2,1.515),(.34,.51,.018),water)
g='freight-crate'
box(g,(0,1.3,0),(2.3,1.3,1.4),paints[3],.06)
for s in [-1,1]:
    for x in range(12):box(g,(-2.1+x*.38,1.3,s*1.41),(.045,1.20,.035),steel,.009)
    for x in [-2.28,2.28]:box(g,(x,1.3,s*1.4),(.07,1.3,.07),ivory)
g='signal-mast'
tube(g,(0,0,0),(0,10,0),.15,steel)
for y in [5,7,9]:
    tube(g,(-1.4,y,0),(1.4,y,0),.07,steel)
    for s in [-1,1]:box(g,(s*1.3,y,0),(.25,.66,.11),ivory)
box(g,(0,.9,0),(.60,.9,.45),dark)

# AURORA: maintained pass infrastructure and a recognizable alpine tree silhouette.
g='snow-fence'
for x in [-2.6,0,2.6]:tube(g,(x,0,0),(x,2.3,0),.09,wood)
for y in [.55,1.0,1.45,1.90]:box(g,(0,y,0),(2.9,.13,.055),wood,.01)
g='weather-station'
crate(g,(0,0,0),(2.4,1.0,1.5),ivory)
tube(g,(0,1,0),(0,5.8,0),.09,steel)
box(g,(0,4.8,0),(.55,.30,.40),orange)
for j in range(3):
    a=j*math.tau/3;tube(g,(0,5.8,0),(math.cos(a)*1.2,5.8,math.sin(a)*1.2),.045,steel)
    ell(g,(math.cos(a)*1.2,5.8,math.sin(a)*1.2),(.20,.12,.20),dark,'Anemometer cup')
box(g,(.0,3.0,.55),(.65,.38,.06),glass)
g='rescue-sled'
for s in [-1,1]:tube(g,(s*.68,.18,-1.7),(s*.68,.35,1.8),.09,steel,'Ski runner')
ell(g,(0,.67,0),(.91,.38,1.70),orange,'Rescue litter')
box(g,(0,.97,0),(.56,.055,1.07),dark,.1)
for z in [-.65,.65]:box(g,(0,1.04,z),(.73,.025,.065),ivory)
g='alpine-pine'
tube(g,(0,0,0),(.12,8.6,0),.19,wood)
for tier in range(6):
    y=2.0+tier*1.02;r=2.15-tier*.29
    for j in range(7):
        a=j*math.tau/7+tier*.42
        tip=(math.cos(a)*r,y-.4,math.sin(a)*r)
        tube(g,(0,y+.55,0),tip,.055,wood)
        panel(g,[(0,y+.9,0),(math.cos(a-.3)*r,y-.25,math.sin(a-.3)*r),(math.cos(a+.3)*r,y-.25,math.sin(a+.3)*r)],.17,leaf,'Needled branch')
        if j%3==0:panel(g,[(0,y+.96,0),(math.cos(a-.22)*r*.85,y-.09,math.sin(a-.22)*r*.85),(math.cos(a+.22)*r*.85,y-.09,math.sin(a+.22)*r*.85)],.065,snow,'Snow resting on branch')

# Physical enamel chips on selected beveled edges, not a blanket rusty overlay.
# Keep the original editable modifiers and add thin, irregular exposed-metal facets.
deps=bpy.context.evaluated_depsgraph_get()
for name,objects in list(groups.items()):
    vertices=[];faces=[]
    probability=.42 if name in ['kart-3','kart-6'] else .045 if name=='kart-5' else .25
    for o in list(objects):
        mode=o.data.materials[0]['runtime'][-1]
        if mode not in [28,31] or not any(m.type=='BEVEL' for m in o.modifiers):continue
        e=o.evaluated_get(deps);mesh=e.to_mesh();mesh.calc_loop_triangles()
        for tri in mesh.loop_triangles:
            # Flat faces stay intact; wear follows the rolled edge and corner geometry.
            if max(abs(k) for k in tri.normal)>.97 or tri.area>.045 or tri.area<1e-8:continue
            center=sum((mesh.vertices[v].co for v in tri.vertices),Vector())/3
            h=math.sin(center.x*183.1+center.y*71.7+center.z*239.3+tri.polygon_index*.37)*43758.5453
            if h-math.floor(h)>probability:continue
            start=len(vertices)
            for vi in tri.vertices:
                q=mesh.vertices[vi].co*.81+center*.19+tri.normal*.0015
                vertices.append(tuple(o.matrix_world@q))
            faces.append((start,start+1,start+2))
        e.to_mesh_clear()
    if vertices:
        mesh=bpy.data.meshes.new('Localized enamel edge chips');mesh.from_pydata(vertices,[],faces)
        o=bpy.data.objects.new('Exposed edge wear',mesh);bpy.context.collection.objects.link(o);add(o,name,steel,'Localized edge chips')

# Timber grain follows each plank's longest local axis, including rotated rails.
def wood_axes(o):
    spans=[max(v.co[k] for v in o.data.vertices)-min(v.co[k] for v in o.data.vertices) for k in range(3)]
    return max(range(3),key=lambda k:spans[k])
def wood_uv(local,normal,major):
    normal_axis=max(range(3),key=lambda k:abs(normal[k]))
    remaining=[k for k in range(3) if k!=major and k!=normal_axis]
    minor=remaining[0]
    if normal_axis==major:
        axes=[k for k in range(3) if k!=major];return (local[axes[0]]*.6,local[axes[1]]*.6)
    return (local[minor]*.6,local[major]*.6)

# Bake proper per-face metric projections after modifiers, not scaled default cube UVs.
deps=bpy.context.evaluated_depsgraph_get()
report={}
for name,objects in groups.items():
    rows=[];bounds=[]
    for o in objects:
        e=o.evaluated_get(deps);mesh=e.to_mesh();mesh.calc_loop_triangles()
        normals=o.matrix_world.to_3x3().inverted().transposed()
        params=list(o.data.materials[0]['runtime'])
        major=wood_axes(o) if params[-1]==29 else None
        for tri in mesh.loop_triangles:
            if tri.area<1e-8:continue
            tn=(normals@tri.normal).normalized();axis=max(range(3),key=lambda k:abs(tn[k]))
            for vi,li in zip(tri.vertices,tri.loops):
                p=o.matrix_world@mesh.vertices[vi].co;n=(normals@mesh.corner_normals[li].vector).normalized()
                if n.length<.5:n=tn
                if o.get('decal'):
                    uv=mesh.uv_layers.active.data[li].uv
                elif major is not None:
                    uv=wood_uv(mesh.vertices[vi].co,tri.normal,major)
                else:
                    # Consistent .6 repeats per meter, fixed to the authored asset.
                    uv=[(p.y*.6,p.z*.6),(p.x*.6,p.z*.6),(p.x*.6,p.y*.6)][axis]
                rows.append((p.x,p.z,-p.y,n.x,n.z,-n.y,*uv,*params))
                bounds.append((p.x,p.z,-p.y))
        e.to_mesh_clear()
    with (OUT/(name+'.qmesh')).open('w') as file:
        file.write('QMS2 '+str(len(rows))+'\n')
        for row in rows:file.write(' '.join(f'{n:.6f}' for n in row)+'\n')
    report[name]={'triangles':len(rows)//3,'materials':sorted(set(o.data.materials[0].name for o in objects)),
                  'min':[min(p[k] for p in bounds) for k in range(3)],'max':[max(p[k] for p in bounds) for k in range(3)]}

# Save actual editable metric UVs into each Blender mesh as well as runtime exports.
for name,objects in groups.items():
    for o in objects:
        if o.type!='MESH' or o.get('decal'):continue
        uv=o.data.uv_layers.active or o.data.uv_layers.new(name='Metric projection 0.6 per meter')
        major=wood_axes(o) if o.data.materials[0]['runtime'][-1]==29 else None
        for poly in o.data.polygons:
            norm=o.matrix_world.to_3x3()@poly.normal;axis=max(range(3),key=lambda k:abs(norm[k]))
            for li in poly.loop_indices:
                p=o.matrix_world@o.data.vertices[o.data.loops[li].vertex_index].co
                uv.data[li].uv=wood_uv(o.data.vertices[o.data.loops[li].vertex_index].co,poly.normal,major) if major is not None else [(p.y*.6,p.z*.6),(p.x*.6,p.z*.6),(p.x*.6,p.y*.6)][axis]

# Keep the source library visually useful: packed paint, wood and livery textures.
paint_path=ROOT/'assets/textures/enamel-wear-v2.png'
decal_path=ROOT/'assets/textures/racer-liveries-v2.png'
wood_path=ROOT/'assets/textures/timber-v2.png'
paint_image=bpy.data.images.load(str(paint_path),check_existing=True)
decal_image=bpy.data.images.load(str(decal_path),check_existing=True)
wood_image=bpy.data.images.load(str(wood_path),check_existing=True) if wood_path.exists() else None
for img in [paint_image,decal_image,wood_image]:
    if img:img.reload()
for material in {o.data.materials[0] for objects in groups.values() for o in objects}:
    rgb=material['runtime'][:3];mode=material['runtime'][-1]
    nodes=material.node_tree.nodes;links=material.node_tree.links;bsdf=nodes.get('Principled BSDF')
    if 15.5<mode<17:
        tex=nodes.new('ShaderNodeTexImage');tex.image=decal_image;tex.extension='EXTEND'
        uv=nodes.new('ShaderNodeTexCoord');mapping=nodes.new('ShaderNodeVectorMath');mapping.operation='MULTIPLY_ADD'
        mapping.inputs[1].default_value=(1,1/8,1);mapping.inputs[2].default_value=(0,(7-round((mode-16)*10))/8,0)
        links.new(uv.outputs['UV'],mapping.inputs[0]);links.new(mapping.outputs[0],tex.inputs['Vector'])
        links.new(tex.outputs['Color'],bsdf.inputs['Base Color']);links.new(tex.outputs['Alpha'],bsdf.inputs['Alpha'])
    elif mode in [28,31]:
        tex=nodes.new('ShaderNodeTexImage');tex.image=paint_image
        strength=nodes.new('ShaderNodeMapRange');strength.inputs['From Min'].default_value=.4;strength.inputs['From Max'].default_value=.8
        strength.inputs['To Min'].default_value=.86;strength.inputs['To Max'].default_value=1.08;links.new(tex.outputs['Color'],strength.inputs['Value'])
        tint=nodes.new('ShaderNodeMixRGB');tint.blend_type='MULTIPLY';tint.inputs[0].default_value=1;tint.inputs[1].default_value=(*rgb,1)
        links.new(strength.outputs['Result'],tint.inputs[2]);links.new(tint.outputs[0],bsdf.inputs['Base Color'])
        bump=nodes.new('ShaderNodeBump');bump.inputs['Strength'].default_value=.08;bump.inputs['Distance'].default_value=.008
        links.new(tex.outputs['Color'],bump.inputs['Height']);links.new(bump.outputs[0],bsdf.inputs['Normal'])
    elif mode==29 and wood_image:
        tex=nodes.new('ShaderNodeTexImage');tex.image=wood_image;links.new(tex.outputs['Color'],bsdf.inputs['Base Color'])
bpy.ops.file.pack_all()
bpy.data.orphans_purge(do_recursive=True)

# Arrange the source library only AFTER local-space runtime exports.
for index,(name,objects) in enumerate(groups.items()):
    for o in objects:o.location+=Vector(((index%7)*12,(index//7)*15,0))
(OUT/'art-v2-manifest.json').write_text(json.dumps(report,indent=2))
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'racers-and-world-v2.blend'))
bpy.ops.export_scene.gltf(filepath=str(OUT/'racers-and-world-v2.glb'),export_format='GLB',export_apply=True)
print(json.dumps({name:info['triangles'] for name,info in report.items()},indent=2))
