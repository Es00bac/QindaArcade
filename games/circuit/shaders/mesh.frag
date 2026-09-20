#version 330 core
in vec3 world,normal,local;in vec2 uv;in vec4 color,mat,shadowCoord;
uniform sampler2D noiseTex,shadowTex,hullTex,grassTex,metalTex,circuitTex,roadTex,woodTex;uniform vec3 eye,hero;uniform float time;uniform int chapter;
out vec4 frag;
in vec3 metric;
const float PI=3.14159265;
float n3(vec3 p){vec3 i=floor(p),f=fract(p);f=f*f*(3-2*f);vec2 uv=i.xy+vec2(37,17)*i.z+f.xy;return mix(texture(noiseTex,(uv+.5)/256.0).r,texture(noiseTex,(uv+vec2(37,17)+.5)/256.0).r,f.z);}
float fbm(vec3 p){return n3(p)*.54+n3(p*2.03)*.27+n3(p*4.07)*.135+n3(p*8.13)*.065;}
float shadow(){vec3 p=shadowCoord.xyz/shadowCoord.w*.5+.5;if(p.z>1.0||p.x<0||p.x>1||p.y<0||p.y>1)return 1.0;float s=0;float bias=max(.0018,.003*(1.0-dot(normalize(normal),normalize(vec3(-.5,.82,.65)))));for(int i=-1;i<=1;i++)for(int j=-1;j<=1;j++)s+=p.z-bias>texture(shadowTex,p.xy+vec2(i,j)/2048.0).r?.26:1.0;return s/9.0;}
vec3 light(vec3 N,vec3 V,vec3 L,vec3 radiance,vec3 base,float rough,float metal){
 vec3 H=normalize(V+L);float NV=max(dot(N,V),.001),NL=max(dot(N,L),.001),NH=max(dot(N,H),0),VH=max(dot(V,H),0);
 float a=rough*rough,a2=a*a,d=NH*NH*(a2-1)+1;float D=a2/(PI*d*d+.0001);float k=(rough+1)*(rough+1)/8;
 float G=NV/(NV*(1-k)+k)*NL/(NL*(1-k)+k);vec3 F=mix(vec3(.045),base,metal)+(1-mix(vec3(.045),base,metal))*pow(1-VH,5);
 return ((1-F)*(1-metal)*base/PI+D*G*F/max(4*NV*NL,.01))*radiance*NL;
}
void main(){
 if(mat.z>11.5 && mat.z<12.5){
  float r=length(local.xz),a=atan(local.z,local.x);float warp=sin(a*4.0-r*12.0+time*.27);
  float strand=pow(.5+.5*sin(a*7.0+pow(r,.6)*24.0-time*.55+warp),15.0);
  float edge=pow(clamp(r,0,1),14);vec3 pool=vec3(.002,.006,.021)+vec3(.035,.17,.32)*strand*r*(1-r)*4;
  pool+=vec3(.015,.25,.45)*edge*.55;pool+=vec3(.05,.024,.1)*pow(1-r,3);
  frag=vec4(pool*color.a,1);return;
 }
 vec3 N=normalize(normal),V=normalize(eye-world),base=color.rgb;
 float rough=mat.x,metal=mat.y,mode=mat.z;float emission=color.a;
 if(mode>=19.5&&mode<23.5){
  vec2 coord=mode<20.5?world.xz*.24:mode>22.5?vec2(uv.x*.60,uv.y*6.5):uv;
  vec3 detail=mode<20.5?texture(grassTex,coord).rgb:mode<21.5?texture(metalTex,coord).rgb:mode<22.5?texture(circuitTex,coord).rgb:texture(roadTex,coord).rgb;
  if(mode>20.5&&mode<21.5){
   // World-space projection keeps broad beams and tiny props at the same texel scale.
   vec3 weights=pow(abs(N),vec3(4));weights/=max(dot(weights,vec3(1)),.001);
   detail=texture(metalTex,world.yz*.45).rgb*weights.x+texture(metalTex,world.xz*.45).rgb*weights.y+texture(metalTex,world.xy*.45).rgb*weights.z;
  }
  if(mode<20.5){
   detail=mix(detail,texture(grassTex,mat2(.8,-.6,.6,.8)*coord*.63+vec2(.37,.19)).rgb,.42);
   vec3 weights=pow(abs(N),vec3(4));weights/=max(dot(weights,vec3(1)),.001);
   detail=detail*weights.y+texture(grassTex,world.yz*.24).rgb*weights.x+texture(grassTex,world.xy*.24).rgb*weights.z;
  }
  if(mode>20.5&&mode<21.5)detail=vec3(.80)+detail*.30;
  base*=detail;rough=mode>22.5?.93:.72;metal=mode>20.5&&mode<22.5?.16:0.0;
  if(mode<20.5){float broad=fbm(world*.018);base*=.72+broad*.60;base=mix(base,vec3(.24,.215,.16)*(.76+broad*.35),clamp(uv.x,0,1)*.75);}
  if(mode>22.5){float border=smoothstep(.445,.48,abs(uv.y-.5));base=mix(base,vec3(.64,.61,.44),border*.78);}
 }
 // Material-specific, object-locked finish. QMS2 exports physical-scale face UVs.
 if(mode>26.5&&mode<31.5){
  float micro=n3(local*63.0);
  if(mode<27.5){base*=.96+micro*.06;rough=max(rough,.66);metal=0.0;}
  else if(mode<28.5||mode>30.5){
   float grain=texture(metalTex,uv).r;
   float chip=(1.0-smoothstep(.39,.49,grain))*(mode>30.5?.12:.85);
   base*=clamp(.92+(grain-.6)*.60,.68,1.08);
   base=mix(base,vec3(.32,.34,.33),chip);rough=clamp(rough+(1-grain)*.15,.28,.85);metal=mix(metal,.72,chip);
   vec3 dx=dFdx(world),dy=dFdy(world),r1=cross(dy,N),r2=cross(N,dx);float det=dot(dx,r1);
   if(abs(det)>.000001)N=normalize(abs(det)*N-sign(det)*.002*(dFdx(grain)*r1+dFdy(grain)*r2));
  }else if(mode<29.5){
   base=mix(texture(woodTex,uv).rgb,base,.30);rough=.81;metal=0.0;
  }else{
   float brushing=.5+.5*sin(uv.y*460.+micro*2.);base*=.985+brushing*.025;rough=clamp(rough+micro*.07,.2,.8);
  }
 }
 if(mode>32.5&&mode<33.5){
  float age=fbm(local*4.2),seam=pow(.5+.5*sin(local.x*23.+fbm(local*2.0)*4.),9.);
  float dirt=(1-smoothstep(.1,.8,local.y))*.20+seam*.05;
  base*=.88+age*.19-dirt;rough=.78;metal=0.0;
 }
 if(mode>24.5&&mode<26.5){
  float stone=fbm(world*.18);base*=.83+stone*.24;rough=.92;
  if(mode>25.5)base=mix(base*.68,vec3(.85,.89,.89),smoothstep(.15,.72,N.y));
 }
 if(mode>12.5&&mode<13.5){
  // Metallic prismatic pavers: dark grout, thin circuit traces, calm emission.
  float h=uv.y*.78+uv.x*.0017+.02*sin(uv.x*.024);
  vec3 spectrum=.48+.45*cos(6.28318*(h+vec3(0,-.333,.333)));
  float cell=fract(uv.x/2.4),crossCell=fract(uv.y*12.0);
  float aa=max(fwidth(cell),.003);
  float seam=1-smoothstep(.011,.011+aa,min(cell,1-cell));
  float across=1-smoothstep(.015,.027,min(crossCell,1-crossCell));
  base=spectrum*.12+vec3(.008,.012,.025);
  base*=1-.70*max(seam,across);
  float edge=smoothstep(.46,.494,abs(uv.y-.5));
  base=mix(base,spectrum*.48+vec3(.009),edge);
  float gridFine=pow(.5+.5*sin(uv.x*9.0),34.0)*.016;
  base+=spectrum*gridFine;
  emission=.26+edge*2.0;rough=.24;metal=.48;
 } else if(mode>13.5&&mode<14.5){
  // Consistent physical facade bays, aligned to each building rather than stretched UVs.
  vec2 grid=vec2((abs(local.x)>abs(local.z)?metric.z:metric.x)/2.7,metric.y/3.4);vec2 cell=fract(grid);
  float seed=fract(sin(dot(floor(grid),vec2(17.17,79.2)))*8193.3);
  float win=step(.28,cell.x)*step(cell.x,.73)*step(.22,cell.y)*step(cell.y,.64)*step(.45,seed)*(1-step(.5,abs(N.y)));
  float glazing=step(.13,cell.x)*step(cell.x,.87)*step(.18,cell.y)*step(cell.y,.78)*(1-step(.5,abs(N.y)));
  vec3 co=mix(vec3(.29,.43,.49),vec3(.70,.61,.39),step(.61,seed));
  base=mix(base,vec3(.085,.125,.15),glazing*.68);base+=co*win*.36;emission=win*.34;rough=.42;metal=.24;
 } else if(mode>14.5&&mode<15.5){
  float tread=step(.24,fract(uv.x*26.+abs(uv.y-.5)*3.5));
  base*=.55+.45*tread;rough=.85;metal=.02;
 }

 if(mode>15.5 && mode<17.0){
  float row=floor((mode-16.0)*10.0+.25);
  vec4 decal=texture(hullTex,vec2(uv.x,(row+1.0-uv.y)/8.0));if(decal.a<.08)discard;base=decal.rgb;rough=.67;metal=0.0;
 }
 if((mode>1.5&&mode<2.5)||(mode>6.5&&mode<7.5)){
  float n=fbm(local*7.0);base*=.50+1.05*n;
  float e=.012;vec3 grad=vec3(fbm((local+vec3(e,0,0))*7)-n,fbm((local+vec3(0,e,0))*7)-n,fbm((local+vec3(0,0,e))*7)-n);
  N=normalize(N-grad*2.1);rough=.87;
  if(mode>6.5){float vein=pow(clamp(1-abs(n-.51)*32,0,1),5);base+=vec3(.32,.1,.007)*vein;emission+=vein*1.6;}
 }else if(mode>4.5&&mode<6.5){
  float n=fbm(local*4);base*=.72+.5*n;
  vec3 irid=.5+.5*cos(vec3(0,2,4)+dot(V,N)*7+local.y*.6);base+=irid*.04;
  if(mode>5.5){float rib=pow(.5+.5*sin(local.z*14+local.x*3),24);float edge=pow(abs(local.z)/4,4);base+=vec3(.012,.08,.1)*rib;emission+=rib*.2+edge*.40;}
 }else if(emission<.1){
  float speck=n3(local*40);base*=.96+speck*.08;
 }
 if(mode>8.5&&mode<9.5){vec4 decal=texture(hullTex,vec2(uv.x,1-uv.y));base=mix(base,decal.rgb,decal.a);rough+=decal.a*.15;}
 base=pow(max(base,vec3(0)),vec3(2.2));
 vec3 result=light(N,V,normalize(vec3(-.50,.82,.65)),vec3(3.3,3.08,2.7),base,rough,metal)*shadow();
 result+=light(N,V,normalize(vec3(.7,.22,-.65)),vec3(.72,.60,1.35),base,rough,metal);
 result+=light(N,V,normalize(vec3(-.6,-.32,.7)),vec3(.13,.23,.35),base,rough,metal);
 float sky=N.y*.5+.5;vec3 ambient=mix(vec3(.11,.13,.17),vec3(.27,.35,.44),sky);
 vec3 R=reflect(-V,N);float fres=pow(1-max(dot(N,V),0),3);
 result+=base*ambient*(1-metal*.6)+ambient*(metal*.26+fres*.12);
 result+=vec3(.03,.22,.17)*base/(1+.17*dot(world-hero,world-hero));
 result+=base*emission;
 bool day=chapter==1||chapter==3;
 if(day)result+=base*vec3(.16,.19,.12);
 float fog=1-exp(-length(world-eye)*(day?.00085:.0011));result=mix(result,day?vec3(.16,.28,.32):vec3(.075,.10,.18),fog);
 frag=vec4(result,1);
}
