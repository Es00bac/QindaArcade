"""Author the reclaimed-world prop kit through Blender MCP; export runtime meshes."""
import bpy
import math
import json
from pathlib import Path
from mathutils import Vector

ROOT = Path('/home/cabewse/work_SPaC3/Games/QindaMegaBrawlSmash')
OUT = ROOT / 'assets/models'
OUT.mkdir(parents=True, exist_ok=True)
# This script runs in the separate factory-startup asset session.
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
groups = {}

def xyz(p):
    return (p[0], -p[2], p[1])

def material(name, color, roughness=.7, metallic=.1):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1)
    mat.use_nodes = True
    shader = mat.node_tree.nodes.get('Principled BSDF')
    shader.inputs['Base Color'].default_value = (*color, 1)
    shader.inputs['Roughness'].default_value = roughness
    shader.inputs['Metallic'].default_value = metallic
    return mat

paint = material('Reclaimed cream enamel', (.68,.67,.56))
glass = material('Smoked CRT glass', (.025,.10,.11), .22, .35)
pcb = material('Aged green circuit boards', (.07,.19,.10))
rubber = material('Old cable insulation', (.035,.04,.037), .9)
leaf = material('Meadow fern', (.17,.32,.055), .82)
atlas = bpy.data.images.load(str(ROOT/'assets/textures/reclaimed-atlas.png'))
for mat, offset in [(pcb, (0, 0))]:
    nodes = mat.node_tree.nodes
    tex = nodes.new('ShaderNodeTexImage'); tex.image = atlas
    coords = nodes.new('ShaderNodeTexCoord')
    mapping = nodes.new('ShaderNodeVectorMath'); mapping.operation = 'MULTIPLY_ADD'
    mapping.inputs[1].default_value = (.485, .485, 0)
    mapping.inputs[2].default_value = (offset[0]+.007, offset[1]+.007, 0)
    repeat = nodes.new('ShaderNodeVectorMath'); repeat.operation = 'FRACTION'
    mat.node_tree.links.new(coords.outputs['UV'], repeat.inputs[0])
    mat.node_tree.links.new(repeat.outputs[0], mapping.inputs[0])
    mat.node_tree.links.new(mapping.outputs[0], tex.inputs['Vector'])
    mat.node_tree.links.new(tex.outputs['Color'], nodes.get('Principled BSDF').inputs['Base Color'])
wear=bpy.data.images.load(str(ROOT/'assets/textures/enamel-wear-v2.png'),check_existing=True)
nodes=paint.node_tree.nodes;links=paint.node_tree.links
tex=nodes.new('ShaderNodeTexImage');tex.image=wear
tint=nodes.new('ShaderNodeMixRGB');tint.blend_type='MULTIPLY';tint.inputs[0].default_value=.20
tint.inputs[1].default_value=(.68,.67,.56,1);links.new(tex.outputs['Color'],tint.inputs[2]);links.new(tint.outputs[0],nodes.get('Principled BSDF').inputs['Base Color'])

def add(obj, group, mat):
    obj.name = group + '/' + obj.name
    obj.data.materials.append(mat)
    groups.setdefault(group, []).append(obj)
    return obj

def box(group, p, half, mat=paint, bevel=.07):
    bpy.ops.mesh.primitive_cube_add(size=2, location=xyz(p))
    obj = bpy.context.object
    obj.scale = (half[0],half[2],half[1])
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if bevel:
        mod = obj.modifiers.new('Manufactured rounded edges', 'BEVEL')
        mod.width=bevel; mod.segments=3
        obj.modifiers.new('Weighted corner normals', 'WEIGHTED_NORMAL')
    return add(obj,group,mat)

def pipe(group, points, radius, mat=paint):
    curve=bpy.data.curves.new('Formed tube','CURVE'); curve.dimensions='3D'
    curve.resolution_u=8; curve.bevel_depth=radius; curve.bevel_resolution=2
    spline=curve.splines.new('POLY'); spline.points.add(len(points)-1)
    for point,p in zip(spline.points,points): point.co=(*xyz(p),1)
    obj=bpy.data.objects.new('Tube',curve); bpy.context.collection.objects.link(obj)
    return add(obj,group,mat)

# Curved CRT body with rear housing, inset screen, grille, feet and knobs.
box('crt-body',(0,.84,0),(1.05,.79,.70),bevel=.16)
box('crt-body',(0,.88,-.63),(.78,.58,.43),bevel=.20)
box('crt-glass',(0,.96,.71),(.85,.57,.085),glass,.13)
box('crt-body',(0,.25,.77),(.89,.09,.04),bevel=.035)
for x in [-.75,.72]:
    box('crt-body',(x,.10,0),(.18,.10,.5),bevel=.04)
for j in range(7):
    box('crt-glass',(.62+j*.045,.26,.817),(.012,.04,.008),glass,.005)
for j in range(3):
    box('crt-glass',(-.70+j*.15,.27,.82),(.055,.045,.025),glass,.025)
for j in range(8):
    box('crt-glass',(1.055,.50+j*.085,-.11),(.008,.015,.28),glass,.003)

# Tall salvaged server and exposed backplane. Shelves make its scale recognizable.
box('server-body',(0,1.55,0),(.75,1.55,.62),bevel=.09)
box('server-face',(0,1.57,.63),(.60,1.36,.035),pcb,.01)
for y in [.35,.83,1.31,1.79,2.27,2.75]:
    box('server-body',(0,y,.69),(.62,.045,.085),bevel=.025)
    for x in [-.47,.47]:
        box('server-body',(x,y+.15,.73),(.03,.075,.035),bevel=.01)

# Thick loose cable coil, useful at the meadow berms and maintenance decks.
for loop in range(5):
    points=[]
    for j in range(65):
        a=j*2*math.pi/64; radius=.62+loop*.13
        points.append((math.cos(a)*radius,.12+loop*.035,math.sin(a)*radius))
    pipe('cable-coil',points,.065,rubber)
pipe('cable-coil',[(.9,.16,.2),(1.5,.10,.4),(1.9,.11,.9),(2.2,.12,1.0)],.065,rubber)
box('cable-coil',(2.3,.14,1),(.16,.11,.10),rubber,.02)

# A salvage-yard gantry: bolted legs, diagonal braces and a hanging cable drum.
for side in [-1,1]:
    box('salvage-gantry',(side*9,5,0),(.42,5,.6),bevel=.10)
    box('salvage-gantry',(side*9,.3,0),(1.2,.3,1.4),bevel=.08)
    pipe('salvage-gantry',[(side*9,7.5,0),(side*6.8,10,0)],.20)
    for h in [1.5,3,4.5,6]:
        box('salvage-gantry',(side*9,h,.65),(.5,.055,.08),bevel=.015)
box('salvage-gantry',(0,10,0),(9.8,.48,.8),bevel=.13)
box('salvage-gantry',(0,9.35,0),(1.05,.30,.72),bevel=.08)
pipe('salvage-gantry',[(0,9.1,0),(0,7.8,0)],.055,rubber)
pipe('salvage-gantry',[(0,7.8,0),(.3,7.65,0),(.40,7.9,0)],.12)

# Fern fronds have curved silhouettes and individual leaflets, not flat billboards.
for frond in range(7):
    a=frond*2*math.pi/7
    points=[]
    for j in range(10):
        t=j/9; r=t*1.1; y=.10+math.sin(t*math.pi*.68)*1.15
        points.append((math.cos(a)*r,y,math.sin(a)*r))
    pipe('fern',points,.014,leaf)
    for j in range(2,9):
        t=j/9; center=Vector(points[j]); tangent=Vector((-math.sin(a),0,math.cos(a)))
        for side in [-1,1]:
            reach=(1-t)*.48
            tip=center+tangent*reach*side+Vector((math.cos(a)*.16,.03,math.sin(a)*.16))
            back=center+Vector((-math.cos(a)*.08,0,-math.sin(a)*.08))
            ahead=center+Vector((math.cos(a)*.12,.025,math.sin(a)*.12))
            mesh=bpy.data.meshes.new('Lanceolate leaflet')
            mesh.from_pydata([xyz(p) for p in [back,tip,ahead,center+Vector((0,.045,0))]],[],[(0,1,3),(1,2,3),(2,0,3)])
            obj=bpy.data.objects.new('Leaflet',mesh);bpy.context.collection.objects.link(obj);add(obj,'fern',leaf)

# Wind-powered reclamation pumps tie the garden and valley to an inhabited world.
pipe('wind-pump',[(0,0,0),(0,10,0)],.22)
box('wind-pump',(0,10,0),(.6,.5,.65),bevel=.18)
for j in range(3):
    a=j*2*math.pi/3
    pipe('wind-pump',[(0,10,.7),(math.sin(a)*3.5,10+math.cos(a)*3.5,.75)],.16)
    pipe('wind-pump',[(math.sin(a)*1.4,10+math.cos(a)*1.4,.75),(math.sin(a)*3.5+.25,10+math.cos(a)*3.5,.75)],.24)

depsgraph=bpy.context.evaluated_depsgraph_get()
report={}
for name,objects in groups.items():
    rows=[]
    for obj in objects:
        evaluated=obj.evaluated_get(depsgraph)
        mesh=evaluated.to_mesh();mesh.calc_loop_triangles()
        normal_matrix=obj.matrix_world.to_3x3().inverted().transposed()
        for tri in mesh.loop_triangles:
            if tri.area<1e-8:continue
            tn=(normal_matrix@tri.normal).normalized();axis=max(range(3),key=lambda k:abs(tn[k]))
            density=1.3 if name=='server-face' else .6
            for vi,li in zip(tri.vertices,tri.loops):
                p=obj.matrix_world@mesh.vertices[vi].co
                n=(normal_matrix@mesh.corner_normals[li].vector).normalized()
                if n.length<.5:n=tn
                uv=[(p.y*density,p.z*density),(p.x*density,p.z*density),(p.x*density,p.y*density)][axis]
                rows.append((p.x,p.z,-p.y,n.x,n.z,-n.y,uv[0],uv[1]))
        evaluated.to_mesh_clear()
    with (OUT/(name+'.qmesh')).open('w') as stream:
        stream.write('QMS1 '+str(len(rows))+'\n')
        for row in rows: stream.write(' '.join(f'{v:.6f}' for v in row)+'\n')
    report[name]={'triangles':len(rows)//3,'source':'Blender MCP 1.5 / Blender '+bpy.app.version_string}

# Match editable mesh UVs to the physical-density runtime projection.
for name,objects in groups.items():
    for obj in objects:
        if obj.type!='MESH':continue
        uv=obj.data.uv_layers.active or obj.data.uv_layers.new(name='Metric face projection')
        density=1.3 if name=='server-face' else .6
        for poly in obj.data.polygons:
            normal=obj.matrix_world.to_3x3()@poly.normal;axis=max(range(3),key=lambda k:abs(normal[k]))
            for li in poly.loop_indices:
                p=obj.matrix_world@obj.data.vertices[obj.data.loops[li].vertex_index].co
                uv.data[li].uv=[(p.y*density,p.z*density),(p.x*density,p.z*density),(p.x*density,p.y*density)][axis]
# Arrange the editable asset library for inspection after exporting local-space meshes.
library_slots = {'crt-body':0, 'crt-glass':0, 'server-body':1, 'server-face':1,
                 'cable-coil':2, 'salvage-gantry':3, 'fern':4, 'wind-pump':5}
for name,objects in groups.items():
    index=library_slots[name]
    for obj in objects: obj.location.x+=(index%4)*24;obj.location.y+=(index//4)*18
bpy.ops.file.pack_all()
bpy.data.orphans_purge(do_recursive=True)
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'reclaimed-world.blend'))
bpy.ops.export_scene.gltf(filepath=str(OUT/'reclaimed-world.glb'),export_format='GLB',export_apply=True)
(OUT/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report))
